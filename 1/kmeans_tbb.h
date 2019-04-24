void compute_k_means( size_t n, const point points[], size_t k, cluster_id id[], point centroid[] ) {

    tls_type tls([&]{return k;}); 
    view global(k);

    // Create initial clusters and compute their sums.
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0,n),
        [=,&tls,&global]( tbb::blocked_range<size_t> r ) {
            view& v = tls.local();
            for( size_t i=r.begin(); i!=r.end(); ++i ) {
                id[i] = i % k;  
                // Peeled "Sum step"
                v.array[id[i]].tally(points[i]);
            }
        }
    );

    // Loop until ids do not change
    size_t change;
    do {
        // Reduce local sums to global sum
        reduce_local_sums_to_global_sum( k, tls, global );

        // Repair any empty clusters
        repair_empty_clusters( n, points, id, k, centroid, global.array );

        // "Divide step": Compute centroids from global sums
        for( size_t j=0; j<k; ++j ) {
            centroid[j] = global.array[j].mean();
            global.array[j].clear();
        }

        // Compute new clusters and their local sums
        tbb::parallel_for(
            tbb::blocked_range<size_t>(0,n),
            [=,&tls,&global]( tbb::blocked_range<size_t> r ) {
                view& v = tls.local();
                for( size_t i=r.begin(); i!=r.end(); ++i ) {
                    // "Reassign step": Find index of centroid closest to points[i]
                    cluster_id j = reduce_min_ind(centroid, k , points[i]); 
                    if( j!=id[i] ) {
                        id[i] = j;
                        ++v.change;
                    }
                    // "Sum step" 
                    v.array[j].tally(points[i]);
                }
            }
        );

        // Reduce local counts to global count
        reduce_local_counts_to_global_count( tls, global );
    } while( global.change!=0 );
}