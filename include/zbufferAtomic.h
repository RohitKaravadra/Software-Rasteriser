#pragma once

#include <concepts>

// Zbuffer class for managing depth values during rendering.
// This class is template-constrained to only work with floating-point types (`float` or `double`).

template<std::floating_point T>		// Restricts T to be a floating-point type
class ZbufferAtomic {
	std::atomic<T>* buffer;			// Pointer to the buffer storing depth values
	unsigned int width, height;		// Dimensions of the Z-buffer

public:
	// Constructor to initialize a Z-buffer with the given width and height.
	// Allocates memory for the buffer.
	// Input Variables:
	// - w: Width of the Z-buffer.
	// - h: Height of the Z-buffer.
	ZbufferAtomic(unsigned int w, unsigned int h) {
		create(w, h);
	}

	// Default constructor for creating an uninitialized Z-buffer.
	ZbufferAtomic() = default;

	// Creates or reinitializes the Z-buffer with the given width and height.
	// Allocates memory for the buffer.
	// Input Variables:
	// - w: Width of the Z-buffer.
	// - h: Height of the Z-buffer.
	void create(unsigned int w, unsigned int h) {
		width = w;
		height = h;
		buffer = new std::atomic<T>[width * height]; // Allocate memory for the buffer
	}

	// Accesses the depth value at the specified (x, y) coordinate.
	// Input Variables:
	// - x: X-coordinate of the pixel.
	// - y: Y-coordinate of the pixel.
	// Returns a reference to the depth value at (x, y).
	T& operator () (unsigned int x, unsigned int y) {
		//return buffer[(y * width) + x]; // Convert 2D coordinates to 1D index
		return buffer[(y * width) + x].load(std::memory_order_relaxed);
	}

	// Get depth value (non-thread-safe for writing)
	T get(unsigned int i) const {
		return buffer[i].load(std::memory_order_relaxed);
	}

	void set(unsigned int i, float val) {
		buffer[i].store(val);
	}

	// Clears the Z-buffer by setting all depth values to 1.0f,
	// which represents the farthest possible depth.
	void clear() {
		for (unsigned int i = 0; i < width * height; ++i) {
			buffer[i].store(1.0f, std::memory_order_relaxed);
		}
	}

	// Destructor to clean up memory allocated for the Z-buffer.
	~ZbufferAtomic() {
		delete[] buffer; // Free the allocated memory
	}
};
