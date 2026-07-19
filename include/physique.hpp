#pragma once

#include <string>
#include <vector>

class Vector2 {

public:
    double x;
    double y;

    Vector2(double x, double y);

    Vector2();

    Vector2 operator+(const Vector2& autre);

    Vector2& operator+=(const Vector2& autre);

    Vector2 operator-(const Vector2& autre);

    Vector2& operator-=(const Vector2& autre);

    Vector2 operator*(double scalaire);

    double norme() const;
};

class Vector3 {

public:
    double x;
    double y;
    double z;

    Vector3(double x, double y, double z);

    Vector3();
};

class Corps {
public:
    std::string nom;
    Vector3 couleur;

    Vector2 force;
    Vector2 position;
    Vector2 vitesse;
    Vector2 acceleration;

    double masse;
    double rayon;
    
    Corps(std::string nom, double x, double y, double vx, double vy, double masse, double rayon, Vector3 couleur): 
          nom(nom), position(x, y), vitesse(vx, vy), masse(masse), rayon(rayon), couleur(couleur) {}
};

class Univers {
    std::vector<Corps>& lsCorps;

public:
    Vector2 vecNul;
    double G;
    int dt;

    Univers(std::vector<Corps>& lsCorps, double G, int dt): lsCorps(lsCorps), G(G), dt(dt) {}  
    
    void calculForces();

    void EulerStep();
    void LeapfrogStep();
};