#include <cstddef>

struct point {
    float x, y;
    void operator+=( const point& right ) {
        x += right.x;
        y += right.y;
    }
    point operator/( size_t count ) const {
        point p = *this;
        p.x /= count;
        p.y /= count;
        return p;
    }
};

inline float distance2( const point& a, const point& b ) {
    float dx = a.x-b.x;
    float dy = a.y-b.y;
    return dx*dx+dy*dy;
}

#include "sum_and_count.h"

typedef unsigned short cluster_id;

void fix_empty_clusters( size_t n, const point points[], cluster_id id[], size_t k, point centroid[], sum_and_count sum[] );

/*
namespace cilk_example {
    void compute_k_means( size_t n, const point points[], size_t k, cluster_id id[], point centroid[] ); 
}
*/

namespace tbb_asura {
    void do_k_means( size_t n, const point points[], size_t k, cluster_id id[], point centroid[] ); 
}
