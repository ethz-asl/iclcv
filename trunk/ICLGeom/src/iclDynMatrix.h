#ifndef ICL_DYN_MATRIX_H
#define ICL_DYN_MATRIX_H
#include <iterator>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>
#include <vector>

#include <iclException.h>

namespace icl{
  struct InvalidMatrixDimensionException :public ICLException{
    InvalidMatrixDimensionException(const std::string &msg):ICLException(msg){}
  };
  struct IncompatibleMatrixDimensionException :public ICLException{
    IncompatibleMatrixDimensionException(const std::string &msg):ICLException(msg){}
  };
  struct  InvalidIndexException : public ICLException{
    InvalidIndexException(const std::string &msg):ICLException(msg){}
  };
  struct SingularMatrixException : public ICLException{
    SingularMatrixException(const std::string &msg):ICLException(msg){}
  };
  
  
  template<class T>
  struct DynMatrix{
    DynMatrix(unsigned int cols,unsigned int rows, T initValue=0) throw (InvalidMatrixDimensionException) : 
    m_rows(rows),m_cols(cols){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      m_data = new T[cols*rows];
      std::fill(begin(),end(),initValue);
    }
    DynMatrix(unsigned int cols,unsigned int rows, T *data, bool deepCopy=true) throw (InvalidMatrixDimensionException) : 
      m_rows(rows),m_cols(cols){
      if(!dim()) throw InvalidMatrixDimensionException("matrix dimensions must be > 0");
      if(deepCopy){
        m_data = new T[dim];
        std::copy(data,data+dim(),begin());
      }else{
        m_data = data;
      }
    }

    DynMatrix(const DynMatrix &other):m_rows(other.m_rows),m_cols(other.m_cols),m_data(new T[dim()]){
      std::copy(other.begin(),other.end(),begin());
    }
    DynMatrix &operator=(const DynMatrix &other){
      if(dim() != other.dim()){
        delete m_data;
        m_data = new T[other.dim()];
      }
      m_cols = other.m_cols;
      m_rows = other.m_rows;

      std::copy(other.begin(),other.end(),begin());
      return *this;
    }
  
  
    DynMatrix operator*(T f) const{
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::multiplies<T>(),f));
      return d;
    }
    DynMatrix &operator*=(T f){
      std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<T>(),f));
      return *this;
    }

    DynMatrix operator/(T f) const{
      return this->operator*(1/4);
    }
    DynMatrix &operator/=(T f){
      return this->operator*=(1/f);
    }

  
    DynMatrix operator*(const DynMatrix &m) const throw (IncompatibleMatrixDimensionException){
      if(m.rows() != cols()) throw IncompatibleMatrixDimensionException("A*B : rows(A) must be cols(B)");
      DynMatrix d(m.cols(),rows());
      for(unsigned int c=0;c<cols();++c){
        for(unsigned int r=0;r<rows();++r){
          d(c,r) = std::inner_product(row_begin(r),row_end(r),m.col_begin(c),0);
        }
      }
      return d;
    }
    DynMatrix &operator*=(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(m.rows() != cols()) throw IncompatibleMatrixDimensionException("A*B : rows(A) must be cols(B)");
      DynMatrix d(m.cols(),rows());
      for(unsigned int c=0;c<cols();++c){
        for(unsigned int r=0;r<rows();++r){
          d(c,r) = std::inner_product(row_begin(r),row_end(r),m.col_begin(c),0);
        }
      }
      return *this=d;
    }
    
    DynMatrix operator/(const DynMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return this->operator*(m.inv());
    }

    DynMatrix &operator/=(const DynMatrix &m) const 
      throw (IncompatibleMatrixDimensionException,
             InvalidMatrixDimensionException,
             SingularMatrixException){
      return *this = this->operator*(m.inv());
    }

    DynMatrix operator+(const T &t){
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::plus<T>(),t));
      return d;
    }
    DynMatrix operator-(const T &t){
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),d.begin(),std::bind2nd(std::minus<T>(),t));
      return d;
    }
    DynMatrix &operator+=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::plus<T>(),t));
      return *this;
    }
    DynMatrix &operator-=(const T &t){
      std::transform(begin(),end(),begin(),std::bind2nd(std::minus<T>(),t));
      return *this;
    }

    DynMatrix operator+(const DynMatrix &m) throw (IncompatibleMatrixDimensionException){
      if(cols() != m.cols() || rows() != m.row()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),m.begin(),d.begin(),std::plus<T>());
      return d;
    }
    DynMatrix operator-(const DynMatrix &m){
      if(cols() != m.cols() || rows() != m.row()) throw IncompatibleMatrixDimensionException("A+B size(A) must be size(B)");
      DynMatrix d(cols(),rows());
      std::transform(begin(),end(),m.begin(),d.begin(),std::minus<T>());
      return d;
    }
    DynMatrix &operator+=(const DynMatrix &m){
      std::transform(begin(),end(),m.begin(),begin(),std::plus<T>());
      return *this;
    }
    DynMatrix &operator-=(const DynMatrix &m){
      std::transform(begin(),end(),m.begin(),begin(),std::minus<T>());
      return *this;
    }


  
    T &operator()(unsigned int col,unsigned int row){
      return m_data[col+cols()*row];
    }
    T operator() (unsigned int col,unsigned int row) const{
      return m_data[col+cols()*row];
    }

    T &at(unsigned int col,unsigned int row) throw (InvalidIndexException){
      if(col>=cols() || row >= rows()) throw InvalidIndexException("row or col index too large");
      return m_data[col+cols()*row];
    }
    T at(unsigned int col,unsigned int row) const throw (InvalidIndexException){
      return const_cast<DynMatrix*>(this)->at(col,row);
    }

    typedef T* iterator;
    typedef const T* const_iterator;

    typedef T* row_iterator;
    typedef const T* const_row_iterator;
  
    unsigned int rows() const { return m_rows; }
  
    unsigned int cols() const { return m_cols; }

    T *data() { return m_data; }

    const T *data() const { return m_data; }

    unsigned int dim() const { return m_rows*m_cols; }
  
    struct col_iterator : public std::iterator<std::random_access_iterator_tag,T>{
      typedef unsigned int difference_type;
      T *p;
      unsigned int stride;
      col_iterator(T *col_begin,unsigned int stride):p(col_begin),stride(stride){}
      col_iterator &operator++(){
        p+=stride;
        return *this;
      }
      col_iterator operator++(int){
        col_iterator tmp = *this;
        ++(*this);
        return tmp;
      }
      col_iterator &operator--(){
        p-=stride;
        return *this;
      }
      col_iterator operator--(int){
        col_iterator tmp = *this;
        --(*this);
        return tmp;
      }
      col_iterator &operator+=(difference_type n){
        p += n * stride;
        return *this;
      }

      col_iterator &operator-=(difference_type n){
        p -= n * stride;
        return *this;
      }
      col_iterator operator+(difference_type n) {
        col_iterator tmp = *this;
        tmp+=n;
        return tmp;
      }
      col_iterator operator-(difference_type n) {
        col_iterator tmp = *this;
        tmp-=n;
        return tmp;
      }
      T &operator*(){
        return *p;
      }
      T operator*() const{
        return *p;
      }

#define X(OP) bool operator OP(const col_iterator &i){ return p OP i.p; }
      X(==)X(<)X(>)X(<=)X(>=)X(!=)
#undef X

      };
  
    typedef const col_iterator const_col_iterator;

    iterator begin() { return m_data; }
    iterator end() { return m_data+dim(); }
  
    const_iterator begin() const { return m_data; }
    const_iterator end() const { return m_data+dim(); }
  
    col_iterator col_begin(unsigned int col) { return col_iterator(m_data+col,cols()); }
    col_iterator col_end(unsigned int col) { return col_iterator(m_data+col+dim(),cols()); }
  
    const_col_iterator col_begin(unsigned int col) const { return col_iterator(m_data+col,cols()); }
    const_col_iterator col_end(unsigned int col) const { return col_iterator(m_data+col+dim(),cols()); }

    row_iterator row_begin(unsigned int row) { return m_data+row*cols(); }
    row_iterator row_end(unsigned int row) { return m_data+(row+1)*cols(); }

    const_row_iterator row_begin(unsigned int row) const { return m_data+row*cols(); }
    const_row_iterator row_end(unsigned int row) const { return m_data+(row+1)*cols(); }



    friend std::ostream &operator<<(std::ostream &s,const DynMatrix &m){
      for(unsigned int i=0;i<m.rows();++i){
        s << "| ";
        std::copy(m.row_begin(i),m.row_end(i),std::ostream_iterator<T>(s," "));
        s << "|" << std::endl;
      }
      return s;
    }

    /// invert the matrix (only implemented with IPP_OPTIMIZATION and only for icl32f and icl64f)
    DynMatrix inv() const throw (InvalidMatrixDimensionException,SingularMatrixException);
  
    DynMatrix transp() const{
      DynMatrix d(rows(),cols());
      for(int i=0;i<cols();++i){
        std::copy(col_begin(i),col_end(i),d.row_begin(i));
      }
      return d;
    }
  private:
    int m_rows;
    int m_cols;
    T *m_data;
  };

}

#endif
