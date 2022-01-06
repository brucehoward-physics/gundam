//
// Created by Nadrino on 26/05/2021.
//

#include "TFile.h"

#include <FitParameter.h>
#include "SplineDial.h"

#include "Logger.h"

LoggerInit([](){ Logger::setUserHeaderStr("[SplineDial]"); } )


SplineDial::SplineDial() {
  SplineDial::reset();
}

void SplineDial::reset() {
  Dial::reset();
  _dialType_ = DialType::Spline;
  _spline_ = TSpline3();
}

void SplineDial::initialize() {
  Dial::initialize();
  LogThrowIf(_spline_.GetXmin() == _spline_.GetXmax(), "Spline is not valid.")

  // check if prior is out of bounds:
  if( _associatedParameterReference_ != nullptr ){
    if(
        static_cast<FitParameter*>(_associatedParameterReference_)->getPriorValue() < _spline_.GetXmin()
      or static_cast<FitParameter*>(_associatedParameterReference_)->getPriorValue() > _spline_.GetXmax()
    ){
      LogError << "Prior value of parameter \""
      << static_cast<FitParameter*>(_associatedParameterReference_)->getTitle()
      << "\" = " << static_cast<FitParameter*>(_associatedParameterReference_)->getPriorValue()
      << " is out of the spline bounds: " <<  _spline_.GetXmin() << " < X < " << _spline_.GetXmax()
      << std::endl;
      throw std::logic_error("Prior is out of the spline bounds.");
    }

    _dialParameterCache_ = static_cast<FitParameter*>(_associatedParameterReference_)->getPriorValue();
    try{ fillResponseCache(); }
    catch(...){
      LogError << "Error while evaluating spline response at the prior value: d(" << _dialParameterCache_ << ") = " << _dialResponseCache_ << std::endl;
      throw std::logic_error("error eval spline response");
    }
  }
  _isInitialized_ = true;
}

std::string SplineDial::getSummary() {
  std::stringstream ss;
  ss << Dial::getSummary();
  return ss.str();
}
void SplineDial::fillResponseCache() {

  if     ( _dialParameterCache_ < _spline_.GetXmin() ) _dialResponseCache_ = _spline_.Eval(_spline_.GetXmin());
  else if( _dialParameterCache_ > _spline_.GetXmax() ) _dialResponseCache_ = _spline_.Eval(_spline_.GetXmax());
  else   {
//    if( fs.stepsize == -1 ){
      _dialResponseCache_ = _spline_.Eval(_dialParameterCache_);
//    }
//    else{
//      fastEval();
//    }
  }
  _minimumSplineResponse_ = 0; // FOR DEBUG
  // Checks
  if( _minimumSplineResponse_ == _minimumSplineResponse_ and _dialResponseCache_ < _minimumSplineResponse_ ){
    _dialResponseCache_ = _minimumSplineResponse_;
  }

  if( _dialResponseCache_ < 0 and _throwIfResponseIsNegative_ ){
    this->writeSpline();
    LogThrow(
      "Negative dial response: dial(" << _dialParameterCache_ << ") = " << _dialResponseCache_
      << std::endl << "Dial is defined in between: [" << _spline_.GetXmin() << ", " << _spline_.GetXmax() << "]" << std::endl
      << ( _associatedParameterReference_ != nullptr ? "Parameter: " + static_cast<FitParameter *>(_associatedParameterReference_)->getName() : "" )
      )
  }
}
void SplineDial::fastEval(){
//Function takes a spline with equidistant knots and the number of steps
  //between knots to evaluate the spline at some position 'pos'.
  fs.l = int((_dialParameterCache_ - _spline_.GetXmin()) / fs.stepsize) + 1;

  _spline_.GetCoeff(fs.l, fs.x, fs.y, fs.b, fs.c, fs.d);
  fs.num = _dialParameterCache_ - fs.x;

  if (fs.num < 0){
    fs.l -= 1;
    _spline_.GetCoeff(fs.l, fs.x, fs.y, fs.b, fs.c, fs.d);
    fs.num = _dialParameterCache_ - fs.x;
  }
  _dialResponseCache_ = (fs.y + fs.num * fs.b + fs.num * fs.num * fs.c + fs.num * fs.num * fs.num * fs.d);
}

void SplineDial::copySpline(const TSpline3* splinePtr_){
  LogThrowIf(_spline_.GetXmin() != _spline_.GetXmax(), "Spline already set")
  _spline_ = *splinePtr_;
}
void SplineDial::createSpline(TGraph* grPtr_){
  LogThrowIf(_spline_.GetXmin() != _spline_.GetXmax(), "Spline already set")
  _spline_ = TSpline3(grPtr_->GetName(), grPtr_);
  fs.stepsize = (_spline_.GetXmax() - _spline_.GetXmin())/((double) grPtr_->GetN());
}
void SplineDial::setMinimumSplineResponse(double minimumSplineResponse) {
  _minimumSplineResponse_ = minimumSplineResponse;
}

const TSpline3* SplineDial::getSplinePtr() const {
  return &_spline_;
}

void SplineDial::writeSpline(const std::string &fileName_) const{
  TFile* f;
  if(fileName_.empty()) f = TFile::Open(Form("badDial_%x.root", this), "RECREATE");
  else                  f = TFile::Open(fileName_.c_str(), "RECREATE");

  f->WriteObject(&_spline_, _spline_.GetName());
  f->Close();
}


