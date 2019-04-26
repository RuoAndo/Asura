void do_k_means( size_t n, const point points[], size_t k, cluster_id id[], point centroid[] ) {

    tls_type tls([&]{return k;}); 
    view global(k);
    
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
    
    size_t change;
    do {
        convert_local_sums_to_global_sum( k, tls, global );
        fix_empty_clusters( n, points, id, k, centroid, global.array );
        
        for( size_t j=0; j<k; ++j ) {
            centroid[j] = global.array[j].mean();
            global.array[j].clear();
        }

        tbb::parallel_for(
            tbb::blocked_range<size_t>(0,n),
            [=,&tls,&global]( tbb::blocked_range<size_t> r ) {
                view& v = tls.local();
                for( size_t i=r.begin(); i!=r.end(); ++i ) {
                    cluster_id j = calc_shortest_index(centroid, k , points[i]); 
                    if( j!=id[i] ) {
                        id[i] = j;
                        ++v.change;
                    }
                    v.array[j].tally(points[i]);
                }
            }
        );
        
        convert_local_counts_to_global_count( tls, global );
    } while( global.change!=0 );
}
