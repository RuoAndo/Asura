class view {
    view( const view& v );            // Deny copy construction
    void operator=( const view& v );  // Deny assignment
public:
    sum_and_count* array;
    size_t change;
    view( size_t k ) : array(new sum_and_count[k]), change(0) {}
    ~view() {delete[] array;}
};

typedef tbb::enumerable_thread_specific<view> tls_type;