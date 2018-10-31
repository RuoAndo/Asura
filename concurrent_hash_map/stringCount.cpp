#include <cstdio>
#include <cctype>
#include <iostream>
#include "tbb/task_scheduler_init.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

using namespace tbb;
using namespace std;

const size_t MAX_N = 1000000;
char data[MAX_N];

struct HashCompare {
    static size_t hash( const char& x ) {
        return (size_t)x;
    }
    static bool equal( const char& x, const char& y ) {
        return x==y;
    }
};

typedef concurrent_hash_map<char, int, HashCompare> StringTable;

struct StringCounter {
    StringTable& table;
    StringCounter( CharTable& table_ ) : table(table_) {}
    void operator()( const blocked_range<char*> range ) const {
        for( char* p=range.begin(); p!=range.end(); ++p ) {
            char c = *p;
            if ( !isalnum(c) )
                continue;
            CharTable::accessor a;
            if ( islower(c) )
                c = toupper(c);
            table.insert( a, c );
            a->second += 1;
        }
    }
};

void CountOccurrences() {
}

int main(int argc, char *argv[])
{
    FILE *fp = fopen("test.txt", "rb");
    if (fp == NULL) {
        cout << "cannot open test.txt" << endl;
        return -1;
    }
    long count=0;
    int b;
    while ( (b = fgetc(fp)) != EOF)
    {
        data[count++] = (char)b;
    }
    fclose(fp);

    std::cout << count << std::endl;
    std::cout << data << std::endl;
    
    task_scheduler_init init;

    CharTable table;
 
    parallel_for( blocked_range<char*>( data, data+count, 100 ), StringCounter(table) );

    for( CharTable::iterator i=table.begin(); i!=table.end(); ++i )
        cout << i->first << "(" << i->second << ") ";
    cout << endl;

    return 0;
}

