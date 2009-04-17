#include "shortpulseinject.h"
#include <cmath>
#include "specfunc.h"

//===============================================================
//==========  ShortPulseInject
//===============================================================

IncidentSourceCurrent *ShortPulseInject::makeECurrent(int distance_, Direction dir_)
{
  std::cerr << "ShortPulseInjectSourceFunc::makeECurrent\n";
  typedef IncidentSourceECurrent<ShortPulseInjectSourceFunc> CurrentType;
  CurrentType *cur = new CurrentType(distance_,dir_);
  
  cur->setParam(length, width, om0, TShift, ZShift, Phase, amp, eps, distance_);
  return cur;
}

IncidentSourceCurrent *ShortPulseInject::makeHCurrent(int distance_, Direction dir_)
{
  std::cerr << "ShortPulseInjectSourceFunc::makeHCurrent\n";
  typedef IncidentSourceHCurrent<ShortPulseInjectSourceFunc> CurrentType;
  CurrentType *cur = new CurrentType(distance_,dir_);
  cur->setParam(length, width, om0, TShift, ZShift, Phase, amp, eps, distance_);
  return cur;
}

bool ShortPulseInject::needCurrent(Direction dir_)
{
  return (dir_ == down);
}
    
ParameterMap* ShortPulseInject::MakeParamMap (ParameterMap* pm)
{
  std::cerr << "ShortPulseInjectSourceFunc::MakeParamMap\n";
  pm = IncidentSource::MakeParamMap(pm);
  
  (*pm)["length"] = WParameter(new ParameterValue<double>(&this->length,1.));
  (*pm)["width"] = WParameter(new ParameterValue<double>(&this->width,1.));
  (*pm)["om0"] = WParameter(new ParameterValue<double>(&this->om0,2*M_PI));
  (*pm)["TShift"] = WParameter(new ParameterValue<double>(&this->TShift,0));
  (*pm)["ZShift"] = WParameter(new ParameterValue<double>(&this->ZShift,0));
  (*pm)["Phase"] = WParameter(new ParameterValue<double>(&this->Phase,0));

  (*pm)["amp"] = WParameter(new ParameterValue<double>(&this->amp,1));
  (*pm)["eps"] = WParameter(new ParameterValue<double>(&this->eps,1));
  
  return pm;
}


//===============================================================
//==========  ShortPulseInjectSourceFunc
//===============================================================

void ShortPulseInjectSourceFunc::setParam(double length_,
                                          double width_,
                                          double om0_,
                                          double TShift_,
                                          double ZShift_,
                                          double Phase_,
                                          double amp_, 
                                          double eps_,
                                          int distance_)
{
  std::cerr << "ShortPulseInjectSourceFunc::setParam\n";
  length = length_;
  width  = width_;
  om0     = om0_;
  TShift  = TShift_;
  ZShift  = ZShift_;
  Phase  = 2*M_PI*Phase_;
    
  amp = amp_;
  eps = eps_;
  dist = distance_;
  
  lightspeed = sqrt(1/eps);
  ZRl = 0.5*om0*width*width/lightspeed;
  
  
  DX = Globals::instance().gridDX();
  DY = Globals::instance().gridDY();
  DZ = Globals::instance().gridDZ();
  DT = Globals::instance().dt() * lightspeed;

  old_time = -1;
  YComp = Complex(0.0,0.0);

  GridIndex gridLow = Globals::instance().gridLow();
  GridIndex gridHigh = Globals::instance().gridHigh();
    
  centrex = 0.5*double(gridHigh[0] + gridLow[0]);
  centrey = 0.5*double(gridHigh[1] + gridLow[1]);
  centrez = 0.5*double(gridHigh[2] + gridLow[2]);
}

void ShortPulseInjectSourceFunc
    ::initSourceFunc(Storage *storage, DataGrid *pJx, DataGrid *pJy, DataGrid *pJz)
{
  std::cerr << "ShortPulseInjectSourceFunc::initSourceFunc\n";
  PsiX  = storage->addBorderLayer("IncidentPsiX" , dir, 4, dist-2, 1);
  PsiY  = storage->addBorderLayer("IncidentPsiY" , dir, 4, dist-2, 1);
  PsiXp = storage->addBorderLayer("IncidentPsiXp", dir, 4, dist-2, 1);
  PsiYp = storage->addBorderLayer("IncidentPsiYp", dir, 4, dist-2, 1);
  std::cerr << "ShortPulseInjectSourceFunc::initSourceFunc ... done\n";
}

void ShortPulseInjectSourceFunc::setTime(int time)
{
  // Now calculating on the fly!
  return;
}

Vector ShortPulseInjectSourceFunc::getEField(int i, int j, int k, int time)
{


  double ex=0, ey=0;
  double posxo = (i-centrex)*DX;
  double posxh = (i+0.5-centrex)*DX;
  double posyo = (j-centrey)*DY;
  double posyh = (j+0.5-centrey)*DY;
  double poszo = (k-centrez)*DZ - ZShift;
  double posTime = time*DT - TShift;

  Complex Exc = Efunc(posxh, posyo, poszo, posTime);
  ex = Exc.real()/M_PI;
    
  if (YComp != Complex(0,0))
  {
    Complex Eyc = YComp*Efunc(posxo, posyh, poszo, posTime);
    ey = Eyc.real()/M_PI;
  }
 
  return Vector(ex,ey,0);

}

Vector ShortPulseInjectSourceFunc::getHField(int i, int j, int k, int time)
{

  double bx=0, by=0;
  double posxo = (i-centrex)*DX;
  double posxh = (i+0.5-centrex)*DX;
  double posyo = (j-centrey)*DY;
  double posyh = (j+0.5-centrey)*DY;
  
//  double poszh = (k+0.5-centrez)*DZ - ZShift;
  double poszh = (k+0.5-centrez)*DZ - ZShift;
//  double posTime = (time+0.5)*DT - TShift;
  double posTime = (time-0.5)*DT - TShift;

  Complex Bxc = Bfunc(posxo, posyh, poszh, posTime, true);
//  Complex Bxc = Bfunc(posxh, posyo, poszh, posTime, true);
  bx = Bxc.real()/M_PI;
  
  Complex Byc = Bfunc(posxh, posyo, poszh, posTime, false);
//  Complex Byc = Bfunc(posxo, posyh, poszh, posTime, false);
  by = Byc.real()/M_PI;

  return Vector(bx,by,0);

}


