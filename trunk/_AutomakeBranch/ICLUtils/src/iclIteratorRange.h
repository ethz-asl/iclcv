#ifndef ICL_ITERATOR_RANGE_H
#define ICL_ITERATOR_RANGE_H

#include <iostream>
#include <algorithm>

namespace icl{
  template<class T, class Iterator>
  struct IteratorRange{
    Iterator begin,end;
    inline IteratorRange(Iterator begin, Iterator end) : begin(begin),end(end){}
    inline IteratorRange(std::pair<Iterator,Iterator> bounds) : begin(bounds.first),end(bounds.second){}

    template<class otherIterator>
    inline IteratorRange &operator=(const IteratorRange<T,otherIterator> &r){
      std::copy(r.begin,r.end,begin);
    }
      
    inline IteratorRange &operator=(const T &value){
      std::fill(begin,end,value);
      return *this;
    }
  };

  template<class T, class Iterator>
  inline std::ostream &operator<<(std::ostream &s, const IteratorRange<T,Iterator> &r){
    s << '[';
    std::copy(r.begin,r.end,std::ostream_iterator<T>(s,","));
    s << ']';
    return s;
  }
  
  
}

#endif
