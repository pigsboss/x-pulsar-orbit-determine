# Introduction #

A software simulates the satellite orbit determination is described in this page.

# Details #

The simulated dynamic model features:
  * Simple [two-body dynamics](TwoBodyDynamics.md),
  * Redundant X-ray pulsars,
  * Dynamics [system error](TwoBodyDynamics.md),
  * TOA measurement error.

The software itself consists of:
  * C/C++ kernel for iterative orbit status estimation
  * Command line user interface
  * OpenGL visualisation for estimated orbit status
  * Text output for estimated orbit status as well as estimate covariance