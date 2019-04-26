void convert_local_counts_to_global_count( tls_type& tls, view& global ) {
    global.change = 0;
    for( auto i=tls.begin(); i!=tls.end(); ++i ) {
        view& v = *i;
        global.change += i->change;
        v.change = 0;
    }
}
