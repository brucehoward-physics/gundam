//
// Created by Adrien BLANCHET on 23/06/2022.
//

#ifndef GUNDAM_JOINTPROBABILITY_H
#define GUNDAM_JOINTPROBABILITY_H

#include "Sample.h"
#include "JsonBaseClass.h"

#include "GenericToolbox.h"

#include "nlohmann/json.hpp"

#include <dlfcn.h>
#include <sstream>
#include <string>
#include <memory>



namespace JointProbability {

  class JointProbability : public JsonBaseClass {
  public:
    // two choices -> either override bin by bin llh or global eval function
    virtual double eval(const Sample& sample_, int bin_){ return 0; }
    virtual double eval(const Sample& sample_){
      double out{0};
      int nBins = int(sample_.getBinning().getBinList().size());
      for( int iBin = 1 ; iBin <= nBins ; iBin++ ){ out += this->eval(sample_, iBin); }
      return out;
    }
  };

  class JointProbabilityPlugin : public JointProbability{

  public:
    double eval(const Sample& sample_, int bin_) override;

    std::string llhPluginSrc;
    std::string llhSharedLib;

  protected:
    void readConfigImpl() override;
    void initializeImpl() override;
    void compile();
    void load();

  private:
    void* fLib{nullptr};
    void* evalFcn{nullptr};

  };


  class Chi2 : public JointProbability{
    double eval(const Sample& sample_, int bin_) override;
  };

  class PoissonLLH : public JointProbability{
    double eval(const Sample& sample_, int bin_) override;
  };

  /// Evaluate the Least Squares difference between the expected and observed.
  /// This is NOT a real LLH function, but is good for debugging since it has
  /// minimal numeric problems (doesn't use any functions like Log or Sqrt).
  class LeastSquaresLLH : public JointProbability{
  protected:
    void readConfigImpl() override;

  public:
    double eval(const Sample& sample_, int bin_) override;

    /// If true the use Poissonian approximation with the variance equal to
    /// the observed value (i.e. the data).
    bool lsqPoissonianApproximation{false};
  };

  class BarlowLLH : public JointProbability{
    double eval(const Sample& sample_, int bin_) override;
  private:
    double rel_var, b, c, beta, mc_hat, chi2;
  };

  class BarlowLLH_BANFF_OA2020 : public JointProbability{
    double eval(const Sample& sample_, int bin_) override;
  };

  class BarlowLLH_BANFF_OA2021 : public JointProbability{

  protected:
    void readConfigImpl() override;

  public:
    double eval(const Sample& sample_, int bin_) override;

    bool verbose{false};
    bool allowZeroMcWhenZeroData{false};
    bool usePoissonLikelihood{false};
    bool BBNoUpdateWeights{false};
  };

  class BarlowLLH_BANFF_OA2021_SFGD : public JointProbability{
    double eval(const Sample& sample_, int bin_) override;
  };

}






#endif //GUNDAM_JOINTPROBABILITY_H