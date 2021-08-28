#include "Image.hh"

template <typename T>
Image<T>::Image(std::size_t width, std::size_t height, T type) : w(width), h(height)
{
}

template <typename T>
Image<T>::~Image()
{
}