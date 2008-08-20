#include <iclFixedMatrix.h>

using namespace icl;

struct F : public FixedMatrix<float,1,1>{
  F():FixedMatrix<float,1,1>(){}
  F(float f):FixedMatrix<float,1,1>(f){
    std::cout << "FixedMatrix constructor called!" << std::endl;
  }
};


int main(){

  std::cout << "#####################\n";
  FixedMatrix<F,1,1> m;
  
  
  
}
