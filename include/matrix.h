#pragma once

#include <iostream>
#include <immintrin.h>
#include <vector>
#include "vec4.h"

// Matrix class for 4x4 transformation matrices
class alignas(16)  matrix {

	union {
		float m[4][4]; // 2D array representation of the matrix
		float a[16];   // 1D array representation of the matrix for linear access
	};
public:

	// Default constructor initializes the matrix as an identity matrix
	matrix() {
		identity();
	}

	// Access matrix elements by row and column
	float& operator()(unsigned int row, unsigned int col) { return m[row][col]; }

	// Display the matrix elements in a readable format
	void display() {
		for (unsigned int i = 0; i < 4; i++) {
			for (unsigned int j = 0; j < 4; j++)
				std::cout << m[i][j] << '\t';
			std::cout << std::endl;
		}
	}

	float& operator[](const int& i)
	{
		return a[i];
	}

	float& operator()(const int& i1, const int& i2)
	{
		return m[i1][i2];
	}

	vec4 mul_point(const vec4& v) const
	{
		vec4 result;
		result[0] = a[0] * v[0] + a[1] * v[1] + a[2] * v[2] + a[3] * v[3];
		result[1] = a[4] * v[0] + a[5] * v[1] + a[6] * v[2] + a[7] * v[3];
		result[2] = a[8] * v[0] + a[9] * v[1] + a[10] * v[2] + a[11] * v[3];
		result[3] = a[12] * v[0] + a[13] * v[1] + a[14] * v[2] + a[15] * v[3];
		return result;
	}

	vec4 mul_point_avx(const vec4& v) const {
		vec4 result;

		// Load the vector `v` into a SIMD register
		__m128 vec = _mm_loadu_ps(v.v);

		// Perform dot products for each row
		__m128 res0 = _mm_dp_ps(_mm_loadu_ps(&a[0]), vec, 0xF1);
		__m128 res1 = _mm_dp_ps(_mm_loadu_ps(&a[4]), vec, 0xF1);
		__m128 res2 = _mm_dp_ps(_mm_loadu_ps(&a[8]), vec, 0xF1);
		__m128 res3 = _mm_dp_ps(_mm_loadu_ps(&a[12]), vec, 0xF1);

		// Store results back into the `result` vector
		result[0] = _mm_cvtss_f32(res0);
		result[1] = _mm_cvtss_f32(res1);
		result[2] = _mm_cvtss_f32(res2);
		result[3] = _mm_cvtss_f32(res3);

		return result;
	}

	// Multiply the matrix by a 4D vector
	// Input Variables:
	// - v: vec4 object to multiply with the matrix
	// Returns the resulting transformed vec4
	vec4 operator * (const vec4& v) const {
		return mul_point_avx(v);
	}

	matrix mul(const matrix& mx) const
	{
		matrix ret;

		// row 0
		ret.a[0] = a[0] * mx.a[0] + a[1] * mx.a[4] + a[2] * mx.a[8] + a[3] * mx.a[12];
		ret.a[1] = a[0] * mx.a[1] + a[1] * mx.a[5] + a[2] * mx.a[9] + a[3] * mx.a[13];
		ret.a[2] = a[0] * mx.a[2] + a[1] * mx.a[6] + a[2] * mx.a[10] + a[3] * mx.a[14];
		ret.a[3] = a[0] * mx.a[3] + a[1] * mx.a[7] + a[2] * mx.a[11] + a[3] * mx.a[15];

		// row 1
		ret.a[4] = a[4] * mx.a[0] + a[5] * mx.a[4] + a[6] * mx.a[8] + a[7] * mx.a[12];
		ret.a[5] = a[4] * mx.a[1] + a[5] * mx.a[5] + a[6] * mx.a[9] + a[7] * mx.a[13];
		ret.a[6] = a[4] * mx.a[2] + a[5] * mx.a[6] + a[6] * mx.a[10] + a[7] * mx.a[14];
		ret.a[7] = a[4] * mx.a[3] + a[5] * mx.a[7] + a[6] * mx.a[11] + a[7] * mx.a[15];

		// row 2
		ret.a[8] = a[8] * mx.a[0] + a[9] * mx.a[4] + a[10] * mx.a[8] + a[11] * mx.a[12];
		ret.a[9] = a[8] * mx.a[1] + a[9] * mx.a[5] + a[10] * mx.a[9] + a[11] * mx.a[13];
		ret.a[10] = a[8] * mx.a[2] + a[9] * mx.a[6] + a[10] * mx.a[10] + a[11] * mx.a[14];
		ret.a[11] = a[8] * mx.a[3] + a[9] * mx.a[7] + a[10] * mx.a[11] + a[11] * mx.a[15];

		// row 3
		ret.a[12] = a[12] * mx.a[0] + a[13] * mx.a[4] + a[14] * mx.a[8] + a[15] * mx.a[12];
		ret.a[13] = a[12] * mx.a[1] + a[13] * mx.a[5] + a[14] * mx.a[9] + a[15] * mx.a[13];
		ret.a[14] = a[12] * mx.a[2] + a[13] * mx.a[6] + a[14] * mx.a[10] + a[15] * mx.a[14];
		ret.a[15] = a[12] * mx.a[3] + a[13] * mx.a[7] + a[14] * mx.a[11] + a[15] * mx.a[15];

		return ret;
	}

	matrix mul_avx(const matrix& mx) const
	{
		matrix ret;

		matrix t = makeTranspose(mx); // transpose matrix b for better memory access

		__m128 ra0, ra1, ra2, ra3; // matrix a resistors
		__m128 rb0, rb1, rb2, rb3; // matrix b resistors

		// load matrix a to resistors
		ra0 = _mm_load_ps(&a[0]); ra1 = _mm_load_ps(&a[4]);
		ra2 = _mm_load_ps(&a[8]); ra3 = _mm_load_ps(&a[12]);

		// load mix b to risistors
		rb0 = _mm_load_ps(&t.a[0]); rb1 = _mm_load_ps(&t.a[4]);
		rb2 = _mm_load_ps(&t.a[8]); rb3 = _mm_load_ps(&t.a[12]);

		// row 0 
		ret.a[0] = _mm_cvtss_f32(_mm_dp_ps(ra0, rb0, 0xF1));
		ret.a[1] = _mm_cvtss_f32(_mm_dp_ps(ra0, rb1, 0xF1));
		ret.a[2] = _mm_cvtss_f32(_mm_dp_ps(ra0, rb2, 0xF1));
		ret.a[3] = _mm_cvtss_f32(_mm_dp_ps(ra0, rb3, 0xF1));

		// row 1
		ret.a[4] = _mm_cvtss_f32(_mm_dp_ps(ra1, rb0, 0xF1));
		ret.a[5] = _mm_cvtss_f32(_mm_dp_ps(ra1, rb1, 0xF1));
		ret.a[6] = _mm_cvtss_f32(_mm_dp_ps(ra1, rb2, 0xF1));
		ret.a[7] = _mm_cvtss_f32(_mm_dp_ps(ra1, rb3, 0xF1));

		// row 2
		ret.a[8] = _mm_cvtss_f32(_mm_dp_ps(ra2, rb0, 0xF1));
		ret.a[9] = _mm_cvtss_f32(_mm_dp_ps(ra2, rb1, 0xF1));
		ret.a[10] = _mm_cvtss_f32(_mm_dp_ps(ra2, rb2, 0xF1));
		ret.a[11] = _mm_cvtss_f32(_mm_dp_ps(ra2, rb3, 0xF1));

		// row 3
		ret.a[12] = _mm_cvtss_f32(_mm_dp_ps(ra3, rb0, 0xF1));
		ret.a[13] = _mm_cvtss_f32(_mm_dp_ps(ra3, rb1, 0xF1));
		ret.a[14] = _mm_cvtss_f32(_mm_dp_ps(ra3, rb2, 0xF1));
		ret.a[15] = _mm_cvtss_f32(_mm_dp_ps(ra3, rb3, 0xF1));

		return ret;
	}

	// Multiply the matrix by another matrix
	// Input Variables:
	// - mx: Another matrix to multiply with
	// Returns the resulting matrix
	matrix operator * (const matrix& mx) const
	{
		return mul_avx(mx);
	}

	static matrix makeTranspose(const matrix& mat)
	{
		matrix t;
		t.a[0] = mat.a[0]; t.a[1] = mat.a[4]; t.a[2] = mat.a[8]; t.a[3] = mat.a[12];
		t.a[4] = mat.a[1]; t.a[5] = mat.a[5]; t.a[6] = mat.a[9]; t.a[7] = mat.a[13];
		t.a[8] = mat.a[2]; t.a[9] = mat.a[6]; t.a[10] = mat.a[10]; t.a[11] = mat.a[14];
		t.a[12] = mat.a[3]; t.a[13] = mat.a[7]; t.a[14] = mat.a[11]; t.a[15] = mat.a[15];
		return t;
	}

	// Create a perspective projection matrix
	// Input Variables:
	// - fov: Field of view in radians
	// - aspect: Aspect ratio of the viewport
	// - n: Near clipping plane
	// - f: Far clipping plane
	// Returns the perspective matrix
	static matrix makePerspective(float fov, float aspect, float n, float f) {
		matrix m;
		m.zero();
		float tanHalfFov = std::tan(fov / 2.0f);

		m.a[0] = 1.0f / (aspect * tanHalfFov);
		m.a[5] = 1.0f / tanHalfFov;
		m.a[10] = -f / (f - n);
		m.a[11] = -(f * n) / (f - n);
		m.a[14] = -1.0f;
		return m;
	}

	// Create a translation matrix
	// Input Variables:
	// - tx, ty, tz: Translation amounts along the X, Y, and Z axes
	// Returns the translation matrix
	static matrix makeTranslation(float tx, float ty, float tz) {
		matrix m;
		m.identity();
		m.a[3] = tx;
		m.a[7] = ty;
		m.a[11] = tz;
		return m;
	}

	// Create a rotation matrix around the Z-axis
	// Input Variables:
	// - aRad: Rotation angle in radians
	// Returns the rotation matrix
	static matrix makeRotateZ(float aRad) {
		matrix m;
		m.identity();
		m.a[0] = std::cos(aRad);
		m.a[1] = -std::sin(aRad);
		m.a[4] = std::sin(aRad);
		m.a[5] = std::cos(aRad);
		return m;
	}

	// Create a rotation matrix around the X-axis
	// Input Variables:
	// - aRad: Rotation angle in radians
	// Returns the rotation matrix
	static matrix makeRotateX(float aRad) {
		matrix m;
		m.identity();
		m.a[5] = std::cos(aRad);
		m.a[6] = -std::sin(aRad);
		m.a[9] = std::sin(aRad);
		m.a[10] = std::cos(aRad);
		return m;
	}

	// Create a rotation matrix around the Y-axis
	// Input Variables:
	// - aRad: Rotation angle in radians
	// Returns the rotation matrix
	static matrix makeRotateY(float aRad) {
		matrix m;
		m.identity();
		m.a[0] = std::cos(aRad);
		m.a[2] = std::sin(aRad);
		m.a[8] = -std::sin(aRad);
		m.a[10] = std::cos(aRad);
		return m;
	}

	// Create a composite rotation matrix from X, Y, and Z rotations
	// Input Variables:
	// - x, y, z: Rotation angles in radians around each axis
	// Returns the composite rotation matrix
	static matrix makeRotateXYZ(float x, float y, float z) {
		return matrix::makeRotateX(x) * matrix::makeRotateY(y) * matrix::makeRotateZ(z);
	}

	// Create a scaling matrix
	// Input Variables:
	// - s: Scaling factor
	// Returns the scaling matrix
	static matrix makeScale(float s) {
		matrix m;
		//s = std::max(s, 0.01f); // Ensure scaling factor is not too small
		s = s > 0.01f ? s : 0.01f;
		m.identity();
		m.a[0] = s;
		m.a[5] = s;
		m.a[10] = s;
		return m;
	}

	// Create an identity matrix
	// Returns an identity matrix
	static matrix makeIdentity() {
		matrix m;
		m.identity();
		return m;
	}

private:
	// Set all elements of the matrix to 0
	void zero() {
		memset(a, 0, 64);
	}

	// Set the matrix as an identity matrix
	void identity() {
		memset(a, 0, 64);
		a[0] = a[5] = a[10] = a[15] = 1;
	}
};


