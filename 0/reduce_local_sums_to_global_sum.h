void convert_local_sums_to_global_sum( size_t k, tls_type& tls, view& global ) {
    for( auto i=tls.begin(); i!=tls.end(); ++i ) {
        view& v = *i;
        for( size_t j=0; j<k; ++j ) {
            global.array[j] += v.array[j];
            v.array[j].clear();
        }
    }
}
