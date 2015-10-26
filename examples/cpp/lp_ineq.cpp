#include <random>
#include <vector>

#include "pogs.h"
#include "timer.h"

// Linear program in inequality form.
//   minimize    c^T * x
//   subject to  Ax <= b.
//
// See <pogs>/matlab/examples/lp_ineq.m for detailed description.
template <typename T>
double LpIneq(size_t m, size_t n) {
  std::vector<T> A(m * n);
  std::vector<T> x(n);
  std::vector<T> y(m);
  std::vector<T> mu(n);
  std::vector<T> nu(m);

  
  std::default_random_engine generator;
  std::uniform_real_distribution<T> u_dist(static_cast<T>(0),
                                           static_cast<T>(1));
  // Generate A according to:
  //   A = [-1 / n *rand(m - n, n); -eye(n)]
  for (unsigned int i = 0; i < (m - n) * n; ++i)
    A[i] = -static_cast<T>(1) / static_cast<T>(n) * u_dist(generator);
  for (unsigned int i = static_cast<unsigned int>((m - n) * n); i < m * n; ++i)
    A[i] = (i - (m - n) * n) % (n + 1) == 0 ? -1 : 0;

  Dense<T, ROW> A_(A.data());
  PogsData<T, Dense<T, ROW>> pogs_data(A_, m, n);
  pogs_data.x = x.data();
  pogs_data.y = y.data();
  pogs_data.nu = nu.data();
  pogs_data.mu = mu.data();


  // Generate b according to:
  //   b = A * rand(n, 1) + 0.2 * rand(m, 1)
  pogs_data.f.reserve(m);
  for (unsigned int i = 0; i < m; ++i) {
    T b_i = static_cast<T>(0);
    for (unsigned int j = 0; j < n; ++j)
      b_i += A[i * n + j] * u_dist(generator);
    b_i += static_cast<T>(0.2) * u_dist(generator);
    pogs_data.f.emplace_back(kIndLe0, static_cast<T>(1), b_i);
  }

  // Generate c according to:
  //   c = rand(n, 1)
  pogs_data.g.reserve(n);
  for (unsigned int i = 0; i < n; ++i)
    pogs_data.g.emplace_back(kIdentity, u_dist(generator));
 
  double t = timer<double>();
  Pogs(&pogs_data);

  return timer<double>() - t;
}

template double LpIneq<double>(size_t m, size_t n);
template double LpIneq<float>(size_t m, size_t n);

