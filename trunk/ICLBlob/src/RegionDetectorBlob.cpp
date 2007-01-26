#include <RegionDetectorBlob.h>
#include <RegionDetectorScanLine.h>
#include <math.h>
#include <Array.h>

namespace icl{
  namespace regiondetector{
    
    int RegionDetectorBlob::s_iReferenceCounter=0;
    
    Array<int> PIXEL_BUFFER;
    
    RegionDetectorBlob::RegionDetectorBlob(RegionDetectorBlobPart *r){
      m_poPixels = new ScanLineList(0);
      if(r){
        fetch(r);
      }
      m_iDirty = 1;

      //s_iReferenceCounter++;
    }
    
    RegionDetectorBlob::~RegionDetectorBlob(){
      delete m_poPixels;
      //s_iReferenceCounter--;
    }

    void RegionDetectorBlob::ensurePixelBufferSize(unsigned int size){
      PIXEL_BUFFER.resize(size);
    }

    int RegionDetectorBlob::size(){
      int s = 0;
      for(ScanLineList::iterator it = m_poPixels->begin(); it!= m_poPixels->end(); it++){
        s += (*it)->size();
      }
      return s;
    }
    
    unsigned char RegionDetectorBlob::val(){
      return m_poPixels->size() ? (*m_poPixels)[0]->val() : 0;
    }  
    
    void RegionDetectorBlob::show(){
      Point p;
      icl8u val;
      getFeatures(p,val);
      printf("RegionDetectorBlob::x=%d y=%d  size=%d  npixellines = %d \n",p.x,p.y,size(),m_poPixels->size());
    }
   
    void RegionDetectorBlob::update(RegionDetectorBlobPart *r){
      m_iDirty = 1;
      m_poPixels->clear();
      fetch(r);
    }
    

    void RegionDetectorBlob::getFeatures(Point &center, icl8u &val){
      if(m_iDirty){
        float tmp_mean_x = 0;
        center.x = 0;
        center.y = 0;
        int sumsize = 0;
        int s = 0;
        val = this->val();
    
        RegionDetectorScanLine *l;
        for(ScanLineList::iterator it = m_poPixels->begin();it!= m_poPixels->end();it++){
          l = *it;
          s = l->size();
          tmp_mean_x += l->x() * s;
          center.y += l->y() * s;
          sumsize += s;
        }
        center.x = (int)round(tmp_mean_x / sumsize);
        center.y = (int)round((float)center.y/sumsize);
        m_oMean = center;
        m_iDirty = 0;
      }else{
        center = m_oMean;
        val = this->val();
      }
    }
    void RegionDetectorBlob::getAllFeatures(const Size &imageSize, Point &center, icl8u &val, Rect &bb,
                                            float &l1, float &l2, float&arc1, float &arc2){

      /*
      int iW, int iH,int &meanx, int &meany, unsigned char &val,  
      int &bbx, int &bby, int &bbw, int &bbh,
      float &l1, float &l2, float&arc1, float &arc2){
      */
      int iW = imageSize.width;
      int iH = imageSize.height;
      //allocating buffer
      
      int *piData=*PIXEL_BUFFER;
  

      if(m_iDirty){
        //pushing pixels into buffer TODO optimize
        int iPos = 0;
        static int iEnd,iY;
        ScanLineList *pll = getPixels();
        for(ScanLineList::iterator it2= pll->begin();it2!=pll->end();it2++){
          RegionDetectorScanLine *pl = (*it2);
          iY=pl->y();
          iEnd = pl->getEnd();
          for(int x=pl->getStart();x<=pl->getEnd();x++){
            piData[iPos++]=x;
            piData[iPos++]=iY;
          }
        }
        
        // calculating features
        int nPts=iPos/2;
    
        static int iCx,iCy;
        static float fL1,fL2,fArc1,fArc2;
        static long iAvgX, iAvgY, iAvgXX, iAvgYY, iAvgXY;
        static double fAvgX, fAvgY;
        static int iXll, iYll, iXur,iYur;
        static float fFac,fSxx,fSyy,fSxy,fP,fD,fA;
        static int n,i,j;
        iCx = 0;
        iXll=iW;
        iYll=0;
        iXur=0;
        iYur=iH;
        //TODO move to Image
        if(!nPts){
          iCx=-1;
          iCy=-1;
          iXll=-1;
          iYll=-1;
          iXur=-1;
          iYur=-1;
          fL1=0.0;
          fL2=0.0;
          fArc1=0.0;
          fArc2=0.0;
        }else{
          //nPts = x
          fFac = 1.0/nPts;
          iAvgX = iAvgY = iAvgXX = iAvgXY = iAvgYY = 0;
          for (n=0; n<2*nPts;) {
            i = piData[n++];
            j = piData[n++];
            iAvgX += i;
            iAvgY += j;
            iAvgXX += i*i;
            iAvgYY += j*j;
            iAvgXY += i*j;
            if(i < iXll)iXll=i;
            if(j > iYll)iYll=j;
            if(i > iXur)iXur=i;
            if(j < iYur)iYur=j;
          }
          fAvgX=((float)iAvgX)*fFac;
          fAvgY=((float)iAvgY)*fFac;

          iCx = (int)round(fAvgX);
          iCy = (int)round(fAvgY);

          fSxx = iAvgXX*fFac - fAvgX*fAvgX;
          fSyy = iAvgYY*fFac - fAvgY*fAvgY;
          fSxy = iAvgXY*fFac - fAvgX*fAvgY;
      
          fP = 0.5*(fSxx+fSyy);
          fD = 0.5*(fSxx-fSyy);
          fD = sqrt(fD*fD + fSxy*fSxy);
          fA  = fP + fD;
          fL1 = (float)(2*sqrt(fP + fD));
          fL2 = (float)(2*sqrt(fP - fD));
          fArc1 = (float)atan2(fA-fSxx,fSxy);
          fArc2 = fArc1+M_PI/2.0;
        }

        //passing output
        center = Point(iCx,iCy);
        //    meanx = iCx;
        //meany = iCy;
        val = this->val();
        //val = nPts ? (*(pll->begin()))->val() : 0;
    
        bb = Rect(iXll,iYur, (iXur-iXll)+1,(iYll-iYur)+1);
        // bbx = iXll;
        // bby = iYur;
        // bbw = (iXur-iXll)+1;
        // bbh = (iYll-iYur)+1;
        l1 = fL1;
        l2 = fL2;
        arc1 = fArc1;
        arc2 = fArc2;
       
        m_oMean = center;
        m_oBB = bb;
        m_afPCA[0] = l1;
        m_afPCA[1] = l2;
        m_afPCA[2] = arc1;
        m_afPCA[3] = arc2;
    
        m_iDirty = 1;
      }else{
        center = m_oMean;
        val = this->val();
        bb = m_oBB;
        l1 = m_afPCA[0];
        l2 = m_afPCA[1];
        arc1 = m_afPCA[2];
        arc2 = m_afPCA[3];
      }
    }
  
    void RegionDetectorBlob::fetch(RegionDetectorBlobPart *r){
      if(r->ps){
        for(ScanLineList::iterator it = r->ps->begin();it!= r->ps->end();++it){
          m_poPixels->push_back(*it);
        }
      }
      if(r->rs){
        for(BlobPartList::iterator it = r->rs->begin();it!= r->rs->end();++it){
          fetch(*it);
        }
      }
    }
  }
}
