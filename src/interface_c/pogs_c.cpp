#include "matrix/matrix_dense.h"
#include "matrix/matrix_sparse.h"
#include "pogs.h"
#include "pogs_c.h"


//Dense Direct
template <typename T, ORD O>
void * PogsInit(size_t m, size_t n, const T *A){
    
    bool directbit = true, densebit = true, rowmajorbit = O == ROW_MAJ;

    char ord = rowmajor ? 'r' : 'c';
    pogs::MatrixDense<T> A_(ord,m,n,A);
    pogs::PogsDirect<T,pogs::MatrixDense<T> > *pogs_data;    
    std::vector<FunctionObj<T> > *f, *g;
    PogsWork<T> * work;


    // create pogs function vectors
    f = new std::vector<FunctionObj<T> >
    g = new std::vector<FunctionObj<T> >


    f->reserve(m);
    for (unsigned int i = 0; i < m; ++i)
      f->emplace_back(static_cast<Function>(kZero), static_cast<T>(1), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));   
 
    g->reserve(n);
    for (unsigned int j = 0; j < n; ++j)
      g->emplace_back(static_cast<Function>(kZero), static_cast<T>(1), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));   


    //create new pogs_data object
    pogs_data = new pogs::PogsDirect<T,pogs::MatrixDense<T> >(A_);

    // create new pogs work struct
    work = new PogsWork<T>(m,n,directbit,densebit,rowmajorbit, static_cast<void *>(pogs_data), static_cast<void *>(f), static_cast<void *>(g));


    return static_cast<void *>(work);

}

//Sparse Indirect
template <typename T, ORD O>
void * PogsInit(size_t m, size_t n, size_t nnz, const T *nzvals, const T *pointers, const T *nzindices){
    
    bool directbit = false, densebit = false, rowmajorbit = O == ROW_MAJ;

    pogs::PogsIndirect<T,pogs::MatrixSparse<T> > *pogs_data;    
    std::vector<FunctionObj<T> > *f, *g;
    PogsWork<T> * work;


    //create pogs_data object
    char ord = rowmajor ? 'r' : 'c';
    pogs::MatrixSparse<T> A_(ord, m, n, nnz, nzvals, pointers, nzindices);
    pogs_data = new pogs::PogsIndirect<T, pogs::MatrixSparse<T> >(A_);


    // create pogs function vectors
    f = new std::vector<FunctionObj<T> >
    g = new std::vector<FunctionObj<T> >

    f->reserve(m);
    for (unsigned int i = 0; i < m; ++i)
      f->emplace_back(static_cast<Function>(kZero), static_cast<T>(1), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));   
 
    g->reserve(n);
    for (unsigned int j = 0; j < n; ++j)
      g->emplace_back(static_cast<Function>(kZero), static_cast<T>(1), static_cast<T>(0), static_cast<T>(1), static_cast<T>(0), static_cast<T>(0));   


    work = new PogsWork<T>(m,n,directbit,densebit,rowmajorbit, static_cast<void *>(pogs_data), static_cast<void *>(f), static_cast<void *>(g));
    return static_cast<void *>(work);
}


template <typename T>
void PogsFunctionUpdate(size_t m, std::vector<FunctionObj<T> > *f, const T *f_a, const T *f_b, const T *f_c, 
                            const T *f_d, const T *f_e, const FUNCTION *f_h){
  for (unsigned int i = 0; i < m; ++i)
    *f[j].a= f_a[i];
  for (unsigned int i = 0; i < m; ++i)
    *f[j].b= f_b[i];
  for (unsigned int i = 0; i < m; ++i)
    *f[j].c= f_c[i];
  for (unsigned int i = 0; i < m; ++i)
    *f[j].d= f_d[i];
  for (unsigned int i = 0; i < m; ++i)
    *f[j].e= f_e[i];
  for (unsigned int i = 0; i < m; ++i)
    *f[j].h= f_h[i];
}

template <typename T>
void PogsRun(pogs::PogsDirect<T, pogs::MatrixDense<T> > &pogs_data, std::vector<FunctionObj<T> > *f, std::vector<FunctionObj<T> > *g, 
              const PogsSettings<T> *settings, PogsInfo<T> *info, PogsSolution<T> *solution){
  // Set parameters.
  pogs_data.SetRho(settings->rho);
  pogs_data.SetAbsTol(settings->abs_tol);
  pogs_data.SetRelTol(settings->rel_tol);
  pogs_data.SetMaxIter(settings->max_iter);
  pogs_data.SetVerbose(settings->verbose);
  pogs_data.SetAdaptiveRho(static_cast<bool>(settings->adaptive_rho));
  pogs_data.SetGapStop(static_cast<bool>(settings->gap_stop));

  // Optionally, feed in warm start variables
  if (static_cast<bool>(settings->warm_start)){
    pogs_data.SetInitX(solution->x);
    pogs_data.SetInitNu(solution->nu);
  }

  // Solve.
  info->status = pogs_data.Solve(*f, *g);

  // Retrieve solver output & state
  info->optval = pogs_data.GetOptval();
  info->iter = pogs_data.GetFinalIter();
  info->rho = pogs_data.GetRho()

  memcpy(solution->x, pogs_data.GetX(), n);
  memcpy(solution->y, pogs_data.GetY(), m);
  memcpy(solution->mu, pogs_data.GetMu(), n);  
  memcpy(solution->nu, pogs_data.GetNu(), m);
}

template<typename T>
void PogsRun(pogs::PogsDirect<T, pogs::MatrixSparse<T> > &pogs_data, std::vector<FunctionObj<T> > *f, std::vector<FunctionObj<T> > *g, 
              const PogsSettings<T> *settings, PogsInfo<T> *info, PogsSolution<T> *solution){
  // Set parameters.
  pogs_data.SetRho(settings->rho);
  pogs_data.SetAbsTol(settings->abs_tol);
  pogs_data.SetRelTol(settings->rel_tol);
  pogs_data.SetMaxIter(settings->max_iter);
  pogs_data.SetVerbose(settings->verbose);
  pogs_data.SetAdaptiveRho(static_cast<bool>(settings->adaptive_rho));
  pogs_data.SetGapStop(static_cast<bool>(settings->gap_stop));

  // Optionally, feed in warm start variables
  if (static_cast<bool>(settings->warm_start)){
    pogs_data.SetInitX(solution->x);
    pogs_data.SetInitNu(solution->nu);
  }

  // Solve.
  info->status = pogs_data.Solve(*f, *g);

  // Retrieve solver output & state
  info->optval = pogs_data.GetOptval();
  info->iter = pogs_data.GetFinalIter();
  info->rho = pogs_data.GetRho()

  memcpy(solution->x, pogs_data.GetX(), n);
  memcpy(solution->y, pogs_data.GetY(), m);
  memcpy(solution->mu, pogs_data.GetMu(), n);  
  memcpy(solution->nu, pogs_data.GetNu(), m);
}

template<typename T>
void PogsRun(pogs::PogsIndirect<T, pogs::MatrixDense<T> > &pogs_data, std::vector<FunctionObj<T> > *f, std::vector<FunctionObj<T> > *g, 
              const PogsSettings<T> *settings, PogsInfo<T> *info, PogsSolution<T> *solution){
  // Set parameters.
  pogs_data.SetRho(settings->rho);
  pogs_data.SetAbsTol(settings->abs_tol);
  pogs_data.SetRelTol(settings->rel_tol);
  pogs_data.SetMaxIter(settings->max_iter);
  pogs_data.SetVerbose(settings->verbose);
  pogs_data.SetAdaptiveRho(static_cast<bool>(settings->adaptive_rho));
  pogs_data.SetGapStop(static_cast<bool>(settings->gap_stop));

  // Optionally, feed in warm start variables
  if (static_cast<bool>(settings->warm_start)){
    pogs_data.SetInitX(solution->x);
    pogs_data.SetInitNu(solution->nu);
  }

  // Solve.
  info->status = pogs_data.Solve(*f, *g);

  // Retrieve solver output & state
  info->optval = pogs_data.GetOptval();
  info->iter = pogs_data.GetFinalIter();
  info->rho = pogs_data.GetRho()

  memcpy(solution->x, pogs_data.GetX(), n);
  memcpy(solution->y, pogs_data.GetY(), m);
  memcpy(solution->mu, pogs_data.GetMu(), n);  
  memcpy(solution->nu, pogs_data.GetNu(), m);
}

template<typename T>
void PogsRun(pogs::PogsIndirect<T, pogs::MatrixSparse<T> > &pogs_data, const std::vector<FunctionObj<T> > *f, std::vector<FunctionObj<T> > *g, 
              const PogsSettings<T> *settings, PogsInfo<T> *info, PogsSolution<T> *solution){
  // Set parameters.
  pogs_data.SetRho(settings->rho);
  pogs_data.SetAbsTol(settings->abs_tol);
  pogs_data.SetRelTol(settings->rel_tol);
  pogs_data.SetMaxIter(settings->max_iter);
  pogs_data.SetVerbose(settings->verbose);
  pogs_data.SetAdaptiveRho(static_cast<bool>(settings->adaptive_rho));
  pogs_data.SetGapStop(static_cast<bool>(settings->gap_stop));
  pogs_data.SetWarmStart(static_cast<bool>(settings->gap_stop));

  // Optionally, feed in warm start variables
  if (static_cast<bool>(settings->warm_start)){
    pogs_data.SetInitX(solution->x);
    pogs_data.SetInitNu(solution->nu);
  }

  // Solve.
  info->status = pogs_data.Solve(*f, *g);

  // Retrieve solver output & state
  info->optval = pogs_data.GetOptval();
  info->iter = pogs_data.GetFinalIter();
  info->rho = pogs_data.GetRho()

  memcpy(solution->x, pogs_data.GetX(), n);
  memcpy(solution->y, pogs_data.GetY(), m);
  memcpy(solution->mu, pogs_data.GetMu(), n);  
  memcpy(solution->nu, pogs_data.GetNu(), m);
}


template <typename T>
int PogsRun(void *work, const T *f_a, const T *f_b, const T *f_c, const T *f_d, const T *f_e, const FUNCTION *f_h,
         const T *g_a, const T *g_b, const T *g_c, const T *g_d, const T *g_e, const FUNCTION *g_h,
         const PogsSettings<T> *settings, PogsInfo<T> *info, PogsSolution<T> *solution){

  p_work = static_cast<PogsWork<T> *>(work);
  
  size_t m = p_work->m;
  size_t n = p_work->n;
  std::vector<FunctionObj<T> > *f = static_cast<std::vector<FunctionObj<T> > *>(p_work->f);
  std::vector<FunctionObj<T> > *g = static_cast<std::vector<FunctionObj<T> > *>(p_work->g);

  // Update f and g
  PogsFunctionUpdate(m, f, f_a, f_b, f_c, f_d, f_e, f_h);
  PogsFunctionUpdate(n, g, g_a, g_b, g_c, g_d, g_e, g_h);

  // Run
  if (p_work->densebit){
    if (p_work->directbit){
      pogs::PogsIndirect<T, pogs::MatrixSparse<T> > *pogs_data = static_cast< pogs::PogsDirect<T, pogs::MatrixDense<T> > >(p_work->pogs_data);
      PogsRun(*pogs_data, f, g, settings_, info_, solution_);
    }else{
      pogs::PogsIndirect<T, pogs::MatrixSparse<T> > *pogs_data = static_cast< pogs::PogsIndirect<T, pogs::MatrixDense<T> > >(p_work->pogs_data);
      PogsRun(*pogs_data, f, g, settings_, info_, solution_);
    }
  }else{
    if (p_work->directbit){
      pogs::PogsIndirect<T, pogs::MatrixSparse<T> > *pogs_data = static_cast< pogs::PogsDirect<T, pogs::MatrixSparse<T> > >(p_work->pogs_data);
      PogsRun(*pogs_data, f, g, settings_, info_, solution_);
    }else{
      pogs::PogsIndirect<T, pogs::MatrixSparse<T> > *pogs_data = static_cast< pogs::PogsIndirect<T, pogs::MatrixSparse<T> > >(p_work->pogs_data);
      PogsRun(*pogs_data, f, g, settings_, info_, solution_);      
    }
  }

  return info->status;
} 

template <typename T>
void PogsShutdown(void * work){
  p_work = static_cast<PogsWork<T> *>(work);
  delete p_work;
}


extern "C" {


void * pogs_init_dense_single(enum ORD ord, size_t m, size_t n, const float *A){
  return ord == COL_MAJ ? return PogsInit<float, COL_MAJ>(m,n,A) : return PogsInit<float, ROW_MAJ>(m,n,A);   
}
void * pogs_init_dense_double(enum ORD ord, size_t m, size_t n, const double *A){
  return ord == COL_MAJ ? PogsInit<double, COL_MAJ>(m,n,A) : PogsInit<double, ROW_MAJ>(m,n,A);   
}
void * pogs_init_sparse_single(enum ORD ord, size_t m, size_t n, size_t nnz, const float *nzvals, const int *indices, const int *pointers){
  return ord == COL_MAJ ? PogsInit<float, COL_MAJ>(m,n,nnz,nzvals,indices,pointers) : PogsInit<float, ROW_MAJ>(m,n,nnz,nzvals,indices,pointers);   
}
void * pogs_init_sparse_double(enum ORD ord, size_t m, size_t n, size_t nnz, const double *nzvals, const int *indices, const int *pointers){
  return ord == COL_MAJ ? PogsInit<double, COL_MAJ>(m,n,nnz,nzvals,indices,pointers) : PogsInit<double, ROW_MAJ>(m,n,nnz,nzvals,indices,pointers);   
}

int pogs_solve_single(void *work, PogsSettings<float> *settings, 
                      const float *f_a, const float *f_b, const float *f_c,const float *f_d, const float *f_e, const enum FUNCTION *f_h,
                      const float *g_a, const float *g_b, const float *g_c,const float *g_d, const float *g_e, const enum FUNCTION *g_h,
                      PogsSolution<float> *solution, PogsInfo<float> *info){
  return PogsRun<float>(work, f_a, f_b, f_c, f_d, f_e, f_h, g_a, g_b, g_c, g_d, g_e, g_h, settings, info, solution);
}
int pogs_solve_double(void *work, PogsSettings<double> *settings, 
                      const double *f_a, const double *f_b, const double *f_c,const double *f_d, const double *f_e, const enum FUNCTION *f_h,
                      const double *g_a, const double *g_b, const double *g_c,const double *g_d, const double *g_e, const enum FUNCTION *g_h,
                      PogsSolution<double> *solution, PogsInfo<double> *info){
  return PogsRun<double>(work, f_a, f_b, f_c, f_d, f_e, f_h, g_a, g_b, g_c, g_d, g_e, g_h, settings, info, solution);
}

void pogs_finish_single(void * work){ return PogsShutdown<float>(work); }
void pogs_finish_double(void * work){ return PogsShutdown<double>(work); }


}

