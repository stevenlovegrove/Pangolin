#pragma once

#include <Eigen/Core>

namespace pangolin
{

enum class Palette : unsigned int {
  none = 0,
  plasma,
  viridis,
  magma,
  inferno,
  turbo,
  coolwarm
};

template <Palette kPalette>
struct Colormap {
  static Eigen::Vector3f color(float t);
  static_assert("Colormap not supported");
};

// plasma, viridis, magma, inferno polynomials originate from here:
// https://www.shadertoy.com/view/3lBXR3
// https://www.shadertoy.com/view/WlfXRN
// Author mattz Licensed as CC0 (public domain)

template <>
struct Colormap<Palette::none> {
  static Eigen::Vector3f color(float t) { return Eigen::Vector3f(t, t, t); }
};

template <>
struct Colormap<Palette::plasma> {
  static Eigen::Vector3f color(float t)
  {
    static const Eigen::Vector3f c0(
        0.05873234392399702, 0.02333670892565664, 0.5433401826748754);
    static const Eigen::Vector3f c1(
        2.176514634195958, 0.2383834171260182, 0.7539604599784036);
    static const Eigen::Vector3f c2(
        -2.689460476458034, -7.455851135738909, 3.110799939717086);
    static const Eigen::Vector3f c3(
        6.130348345893603, 42.3461881477227, -28.51885465332158);
    static const Eigen::Vector3f c4(
        -11.10743619062271, -82.66631109428045, 60.13984767418263);
    static const Eigen::Vector3f c5(
        10.02306557647065, 71.41361770095349, -54.07218655560067);
    static const Eigen::Vector3f c6(
        -3.658713842777788, -22.93153465461149, 18.19190778539828);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
  }
};

template <>
struct Colormap<Palette::viridis> {
  static Eigen::Vector3f color(float t)
  {
    static const Eigen::Vector3f c0(
        0.2777273272234177, 0.005407344544966578, 0.3340998053353061);
    static const Eigen::Vector3f c1(
        0.1050930431085774, 1.404613529898575, 1.384590162594685);
    static const Eigen::Vector3f c2(
        -0.3308618287255563, 0.214847559468213, 0.09509516302823659);
    static const Eigen::Vector3f c3(
        -4.634230498983486, -5.799100973351585, -19.33244095627987);
    static const Eigen::Vector3f c4(
        6.228269936347081, 14.17993336680509, 56.69055260068105);
    static const Eigen::Vector3f c5(
        4.776384997670288, -13.74514537774601, -65.35303263337234);
    static const Eigen::Vector3f c6(
        -5.435455855934631, 4.645852612178535, 26.3124352495832);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
  }
};

template <>
struct Colormap<Palette::magma> {
  static Eigen::Vector3f color(float t)
  {
    static const Eigen::Vector3f c0(
        -0.002136485053939582, -0.000749655052795221, -0.005386127855323933);
    static const Eigen::Vector3f c1(
        0.2516605407371642, 0.6775232436837668, 2.494026599312351);
    static const Eigen::Vector3f c2(
        8.353717279216625, -3.577719514958484, 0.3144679030132573);
    static const Eigen::Vector3f c3(
        -27.66873308576866, 14.26473078096533, -13.64921318813922);
    static const Eigen::Vector3f c4(
        52.17613981234068, -27.94360607168351, 12.94416944238394);
    static const Eigen::Vector3f c5(
        -50.76852536473588, 29.04658282127291, 4.23415299384598);
    static const Eigen::Vector3f c6(
        18.65570506591883, -11.48977351997711, -5.601961508734096);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
  }
};

template <>
struct Colormap<Palette::inferno> {
  static Eigen::Vector3f color(float t)
  {
    static const Eigen::Vector3f c0(
        0.0002189403691192265, 0.001651004631001012, -0.01948089843709184);
    static const Eigen::Vector3f c1(
        0.1065134194856116, 0.5639564367884091, 3.932712388889277);
    static const Eigen::Vector3f c2(
        11.60249308247187, -3.972853965665698, -15.9423941062914);
    static const Eigen::Vector3f c3(
        -41.70399613139459, 17.43639888205313, 44.35414519872813);
    static const Eigen::Vector3f c4(
        77.162935699427, -33.40235894210092, -81.80730925738993);
    static const Eigen::Vector3f c5(
        -71.31942824499214, 32.62606426397723, 73.20951985803202);
    static const Eigen::Vector3f c6(
        25.13112622477341, -12.24266895238567, -23.07032500287172);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
  }
};

// https://ai.googleblog.com/2019/08/turbo-improved-rainbow-colormap-for.html
// Google's perceptually more linear veresion of jet
// Polynomial fit by mattz from https://www.shadertoy.com/view/3lBXR3
template <>
struct Colormap<Palette::turbo> {
  static Eigen::Vector3f color(float t)
  {
    static const Eigen::Vector3f c0(
        0.1140890109226559, 0.06288340699912215, 0.2248337216805064);
    static const Eigen::Vector3f c1(
        6.716419496985708, 3.182286745507602, 7.571581586103393);
    static const Eigen::Vector3f c2(
        -66.09402360453038, -4.9279827041226, -10.09439367561635);
    static const Eigen::Vector3f c3(
        228.7660791526501, 25.04986699771073, -91.54105330182436);
    static const Eigen::Vector3f c4(
        -334.8351565777451, -69.31749712757485, 288.5858850615712);
    static const Eigen::Vector3f c5(
        218.7637218434795, 67.52150567819112, -305.2045772184957);
    static const Eigen::Vector3f c6(
        -52.88903478218835, -21.54527364654712, 110.5174647748972);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
  }
};

template <>
struct Colormap<Palette::coolwarm> {
  static Eigen::Vector3f color(float t)
  {
    static const Eigen::Vector3f c0(0.227376, 0.286898, 0.752999);
    static const Eigen::Vector3f c1(1.204846, 2.314886, 1.563499);
    static const Eigen::Vector3f c2(0.102341, -7.369214, -1.860252);
    static const Eigen::Vector3f c3(2.218624, 32.578457, -1.643751);
    static const Eigen::Vector3f c4(-5.076863, -75.374676, -3.704589);
    static const Eigen::Vector3f c5(1.336276, 73.453060, 9.595678);
    static const Eigen::Vector3f c6(0.694723, -25.863102, -4.558659);
    return c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6)))));
  }
};

}  // namespace pangolin
