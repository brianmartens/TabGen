#ifndef QUICKSORT_H_INCLUDED
#define QUICKSORT_H_INCLUDED
#include <vector>

int partition(std::vector<double> *arr, const int left, const int right)
{
    const int mid = left + (right-left)/2;
    const double pivot = arr->at(mid);
    std::swap(arr->at(mid),arr->at(left));
    int i = left + 1;
    int j = right;
    while(i <= j)
    {
        while(i <= j && arr->at(i) <= pivot){
            ++i;
        }

        while(i <= j && arr->at(j) > pivot){
            --j;
        }

        if(i < j){
            std::swap(arr->at(i),arr->at(j));
        }
    }
    std::swap(arr->at(i-1), arr->at(left));
    return i-1;
}
void quicksort(std::vector<double> *input, int left, int right)
{
    if(left >= right){
        return;
    }

    int part = partition(input, left, right);

    quicksort(input, left, part - 1);
    quicksort(input, part + 1, right);
}

#endif // QUICKSORT_H_INCLUDED
