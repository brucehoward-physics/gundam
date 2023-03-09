//
// Created by Adrien BLANCHET on 02/12/2021.
//

#ifndef GUNDAM_GRAPHDIAL_H
#define GUNDAM_GRAPHDIAL_H

#include "TGraph.h"

#include "Dial.h"

#ifdef USE_NEW_DIALS
#warning Not used with new dial implementation
#endif

class GraphDial : public Dial {

public:
  explicit GraphDial(const DialSet* owner_);

  [[nodiscard]] std::unique_ptr<Dial> clone() const override { return std::make_unique<GraphDial>(*this); }

  void setGraph(const TGraph &graph);

  void initialize() override;

  double calcDial(double parameterValue_) override;
  std::string getSummary() override;

private:
  TGraph _graph_;
};


#endif //GUNDAM_GRAPHDIAL_H
