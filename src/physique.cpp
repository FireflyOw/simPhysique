#include "physique.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

Vector2::Vector2(double x, double y) {
    this->x = x;
    this->y = y;
}

Vector2::Vector2() {
    this->x = 0;
    this->y = 0;
}

Vector2 Vector2::operator+(const Vector2& autre) {
    double nx = x + autre.x;
    double ny = y + autre.y;

    return Vector2(nx, ny);
}

Vector2& Vector2::operator+=(const Vector2& autre) {
    x += autre.x;
    y += autre.y;

    return *this;
}

Vector2 Vector2::operator-(const Vector2& autre) {
    double nx = x - autre.x;
    double ny = y - autre.y;

    return Vector2(nx, ny);
}

Vector2& Vector2::operator-=(const Vector2& autre) {
    x -= autre.x;
    y -= autre.y;

    return *this;
}

Vector2 Vector2::operator*(double scalaire) {
    double nx = x * scalaire;
    double ny = y * scalaire;

    return Vector2(nx, ny);
}

double Vector2::norme() const {
    return std::sqrt(x * x + y * y);
}

Vector3::Vector3(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vector3::Vector3() {
    this->x = 0;
    this->y = 0;
    this->z = 0;
}
    
void Univers::calculForces() {
    for (Corps& corps : lsCorps) {
        corps.force = vecNul;
    }

    for (size_t i = 0; i < lsCorps.size(); i++) {
        for (size_t j = i + 1; j < lsCorps.size(); j++) {
            Vector2 vecDistance;
            vecDistance = lsCorps[i].position - lsCorps[j].position;

            Vector2 vecUnitaire;
            vecUnitaire = vecDistance * (1 / vecDistance.norme());

            double distance;
            distance = vecDistance.norme();

            lsCorps[j].force += vecUnitaire * G * lsCorps[i].masse * lsCorps[j].masse * (1 / (distance * distance));
            lsCorps[i].force += (vecNul - vecUnitaire) * G * lsCorps[i].masse * lsCorps[j].masse * (1 / (distance * distance));

        } 
    }
}

void Univers::EulerStep() {
    calculForces();

    for (Corps& corps : lsCorps) {
        corps.acceleration = corps.force * (1 / corps.masse);

        corps.vitesse += corps.acceleration * dt;
        corps.position += corps.vitesse * dt;
    }
}

void Univers::LeapfrogStep() {
    calculForces();

    for (Corps& corps : lsCorps) {
        corps.acceleration = corps.force * (1 / corps.masse);

        corps.vitesse += corps.acceleration * (dt / 2.0f);
        corps.position += corps.vitesse * dt;
    }

    calculForces();

    for (Corps& corps : lsCorps) {
        corps.acceleration = corps.force * (1 / corps.masse);

        corps.vitesse += corps.acceleration * (dt / 2.0f);
    }
}