// Structure that holds a sum of points and the number of points 
// accumulated into that sum, so that the mean can be computed.
struct sum_and_count {
    sum_and_count() : sum(), count(0) {}
    point sum;
    size_t count;
    void clear() {
        sum = point();
        count = 0;
    }
    void tally( const point& p ) {
        sum += p;
        ++count;
    }
    point mean() const {
        return sum/count;
    }
    void operator+=( const sum_and_count& other ) {
        sum += other.sum;
        count += other.count;
    };
};