int calc_shortest_index( const point centroid[], size_t k, point value ) { 
    int min = -1;
    float mind = std::numeric_limits<float>::max();
    for( int j=0; j<k; ++j ) {
        float d = distance2(centroid[j],value);
        if( d<mind  ) {
            mind = d;
            min = j;
        }
    }
    return min;
}
