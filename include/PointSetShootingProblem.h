#ifndef POINTSETSHOOTINGPROBLEM_H_
#define POINTSETSHOOTINGPROBLEM_H_

#include <iostream>
#include <stdio.h>
#include <algorithm>

#include <vnl/vnl_cost_function.h>

#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkQuadricClustering.h>
#include <vtkSmartPointer.h>

using namespace std;

#include <cuda.h>
#include <cuda_runtime.h>

#include "helper_functions.h"
#include "helper_cuda.h"

struct ShootingParameters{
	enum Algorithm { Allassonniere, GradDescent, QuasiAllassonniere };
	string fnTemplate, fnTarget;
	string fnOutput;
	string fnOutputPaths;
	double sigma;
	double lambda;
	double downsample;
	unsigned int dim;
	unsigned int N;
	unsigned int iter_grad, iter_newton;
	Algorithm alg;

	std::vector<int> qcdiv;

	ShootingParameters() :
		N(100), dim(3), sigma(0.0), lambda(0.0), iter_grad(20), iter_newton(20), downsample(1), alg(GradDescent) {}
};


// Note that in this class all pointers refer to device data! Don't use any host code on them!
struct ShootingData{
	int np;
	int k;
	int VDIM;
	int N;

	std::vector<int> index;

	float * H_iter;

	// 2D variable
	float2 * q02d;
	float2 * p02d;
	float2 * q12d;
	float2 * p12d;
	float2 * qT2d;

	float2 * hq2d;
	float2 * hp2d;
	float2 * grad2d;
	float * grad_linear_2d;

	float * x2d;

	float2 * alpha2d;
	float2 * beta2d;

	float2 * dalpha2d;
	float2 * dbeta2d;

	std::vector<float2*> Qt2d;
	std::vector<float2*> Pt2d;

	// 3D variable
	float3 * q03d;
	float3 * p03d;
	float3 * q13d;
	float3 * p13d;
	float3 * qT3d;

	float3 * hq3d;
	float3 * hp3d;
	float3 * grad3d;
	float * grad_linear_3d;

	float * x3d;

	float3 * alpha3d;
	float3 * beta3d;

	float3 * dalpha3d;
	float3 * dbeta3d;

	std::vector<float3*> Qt3d;
	std::vector<float3*> Pt3d;
};

/* VNL Cost function class */
class PointSetShootingCostFunction_CUDA : public vnl_cost_function{
public:
	PointSetShootingCostFunction_CUDA(const ShootingParameters &param, const ShootingData &data);
	virtual void compute(vnl_vector<double> const& x, double *f, vnl_vector<double>* g);
	void compute2D(vnl_vector<double> const& x, double *f, vnl_vector<double>* g);
	void compute3D(vnl_vector<double> const& x, double *f, vnl_vector<double>* g);

	double gradient_time;
protected:
	ShootingParameters param;
	ShootingData data;
};

class PointSetShootingTransversalityCostFunction_CUDA : public vnl_cost_function{
public:
	PointSetShootingTransversalityCostFunction_CUDA(const ShootingParameters &param, const ShootingData &data);
	virtual void compute(vnl_vector<double> const& x, double *f, vnl_vector<double>* g);
	void compute3D(vnl_vector<double> const& x, double *f, vnl_vector<double>* g);
	void compute2D(vnl_vector<double> const& x, double *f, vnl_vector<double>* g);
protected:
	ShootingParameters param;
	ShootingData data;
};


/* Actual class */
class PointSetShootingProblem_CUDA{
public:
	// Minimize using the transversality principle
	// Note that this is similar to newton method, expensive to run
	// TODO: implement this if necessary
	static void minimize_Allassonniere(const ShootingParameters &param, const ShootingData &data);

	// This quasi method is not so expensive
	static void minimize_QuasiAllassonniere(const ShootingParameters &param, const ShootingData &data);

	// Minimize using gradient descent
	static void minimize_gradient(const ShootingParameters &param, const ShootingData &data);

	// Auxillary class
	static void ReadVTKPointSetWithSampling( const ShootingParameters &param, ShootingData &data, vtkPolyData * pTemplate, vtkPolyData *pTarget);

	static void AllocateCUDAMemAndCopyData(const ShootingParameters &param, ShootingData &data, vtkPolyData * pTemplate, vtkPolyData *pTarget);

	static void WriteVTKPOintSet( const ShootingParameters &param, ShootingData &data, vtkPolyData * pTemplate, vtkPolyData *pTarget );

	static void FreeCUDAMem(const ShootingParameters &param, ShootingData &data);

	// The main function that chains up all the parts
	static void shooting(const ShootingParameters &param);

private:
};

#endif
