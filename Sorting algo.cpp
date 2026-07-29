#include <stdio.h>      
#include <stdlib.h>     
#include <time.h>       

long long comparisons = 0;
long long swaps = 0;

// Resets counters before each algorithm run so results aren't cumulative
void resetCounters() {
    comparisons = 0;
    swaps = 0;
}

// Simple swap helper - also increments the global swap counter
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
    swaps++;              // count every swap performed
}

// Fills an array with random integers in [0, 99999]
void generateRandomArray(int arr[], int n) {
    for (int i = 0; i < n; i++)
        arr[i] = rand() % 100000;
}

// Makes an exact copy of array 'src' into 'dest' (so all 3 algorithms
// sort the SAME data set -> fair comparison)
void copyArray(int dest[], int src[], int n) {
    for (int i = 0; i < n; i++)
        dest[i] = src[i];
}

// Utility to print an array (only used for small test sizes/debugging)
void printArray(int arr[], int n) {
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}


// Merges two sorted subarrays arr[left..mid] and arr[mid+1..right]
void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;      // size of left subarray
    int n2 = right - mid;         // size of right subarray

    // Temporary arrays (this is the O(n) extra space Merge Sort needs)
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;   // i -> L index, j -> R index, k -> arr index

    // Merge the two temp arrays back into arr[left..right] in sorted order
    while (i < n1 && j < n2) {
        comparisons++;             // one comparison per loop iteration
        if (L[i] <= R[j]) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }

    // Copy any remaining elements of L[] (if R[] was exhausted first)
    while (i < n1) arr[k++] = L[i++];

    // Copy any remaining elements of R[] (if L[] was exhausted first)
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

// Recursively splits the array and merges sorted halves
// Recurrence: T(n) = 2T(n/2) + O(n)  =>  O(n log n) in ALL cases
void mergeSort(int arr[], int left, int right) {
    if (left < right) {                     // base case: single element -> already sorted
        int mid = left + (right - left) / 2; // avoids overflow vs (left+right)/2

        mergeSort(arr, left, mid);          // sort left half     -> T(n/2)
        mergeSort(arr, mid + 1, right);     // sort right half    -> T(n/2)
        merge(arr, left, mid, right);       // merge both halves  -> O(n)
    }
}


// Lomuto partition scheme: uses the LAST element as pivot
int partition(int arr[], int low, int high) {
    int pivot = arr[high];   // choosing last element as pivot
    int i = low - 1;         // index of last element smaller than pivot

    for (int j = low; j < high; j++) {
        comparisons++;                 // comparing arr[j] with pivot
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);    // move smaller element to the left side
        }
    }
    swap(&arr[i + 1], &arr[high]);     // place pivot in its correct sorted position
    return i + 1;                      // return pivot's final index
}

// Recurrence (average case, balanced split): T(n) = 2T(n/2) + O(n) -> O(n log n)
// Recurrence (worst case, sorted input)    : T(n) = T(n-1) + O(n)  -> O(n^2)
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);   // partition index

        quickSort(arr, low, pi - 1);          // sort elements before pivot
        quickSort(arr, pi + 1, high);         // sort elements after pivot
    }
}


// Shifts the element at index i DOWN so the subtree rooted at i satisfies
// the max-heap property (parent >= children). 'n' is the current heap size.
void heapify(int arr[], int n, int i) {
    int largest = i;        // assume root (i) is largest initially
    int left = 2 * i + 1;   // left child index
    int right = 2 * i + 2;  // right child index

    if (left < n) {
        comparisons++;
        if (arr[left] > arr[largest])
            largest = left;
    }

    if (right < n) {
        comparisons++;
        if (arr[right] > arr[largest])
            largest = right;
    }

    // If a child is bigger than the parent, swap and continue sifting down
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest);   // recursively fix the affected subtree
    }
}

// Builds max-heap (O(n)) then does n extract-max operations (O(log n) each)
// Total: O(n) + O(n log n) = O(n log n)
void heapSort(int arr[], int n) {
    // Step 1: Build max heap - start from last non-leaf node, go up to root
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // Step 2: Extract elements one by one from the heap
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);    // move current max (root) to the end
        heapify(arr, i, 0);        // restore heap property on reduced heap (size i)
    }
}


int main() {
    srand(time(NULL));   // seed RNG once, based on current time

    // Test with several input sizes to observe how time scales with n
    int sizes[] = {1000, 5000, 10000, 20000, 40000};
    int numSizes = sizeof(sizes) / sizeof(sizes[0]);

    for (int s = 0; s < numSizes; s++) {
        int n = sizes[s];

        // Allocate original + 3 working copies (one per algorithm)
        int *original = (int *)malloc(n * sizeof(int));
        int *arr1 = (int *)malloc(n * sizeof(int));  // for merge sort
        int *arr2 = (int *)malloc(n * sizeof(int));  // for quick sort
        int *arr3 = (int *)malloc(n * sizeof(int));  // for heap sort

        generateRandomArray(original, n);
        copyArray(arr1, original, n);
        copyArray(arr2, original, n);
        copyArray(arr3, original, n);

        clock_t start, end;   // clock_t stores clock ticks

        // ---------------- MERGE SORT ----------------
        resetCounters();
        start = clock();                       // record ticks before sorting
        mergeSort(arr1, 0, n - 1);
        end = clock();                         // record ticks after sorting
        printf("Merge Sort   %6d   %10ld   %10.6f   %10lld   %6lld\n",
               n, (long)(end - start),
               (double)(end - start) / CLOCKS_PER_SEC,
               comparisons, swaps);

        // ---------------- QUICK SORT ----------------
        resetCounters();
        start = clock();
        quickSort(arr2, 0, n - 1);
        end = clock();
        printf("Quick Sort   %6d   %10ld   %10.6f   %10lld   %6lld\n",
               n, (long)(end - start),
               (double)(end - start) / CLOCKS_PER_SEC,
               comparisons, swaps);

        // ---------------- HEAP SORT ----------------
        resetCounters();
        start = clock();
        heapSort(arr3, n);
        end = clock();
        printf("Heap Sort    %6d   %10ld   %10.6f   %10lld   %6lld\n",
               n, (long)(end - start),
               (double)(end - start) / CLOCKS_PER_SEC,
               comparisons, swaps);


        free(original);
        free(arr1);
        free(arr2);
        free(arr3);
    }

    printf("CLOCKS_PER_SEC on this system = %ld\n", (long)CLOCKS_PER_SEC);

    return 0;
}


/* Output for this system:

Merge Sort     1000            0     0.000000         8711        0
Quick Sort     1000            0     0.000000        11061     6013
Heap Sort      1000            0     0.000000        16821     9062
Merge Sort     5000            0     0.000000        55242        0
Quick Sort     5000            0     0.000000        74768    39598
Heap Sort      5000            0     0.000000       107562    57098
Merge Sort    10000            0     0.000000       120499        0
Quick Sort    10000            0     0.000000       156851    91218
Heap Sort     10000            0     0.000000       235454   124194
Merge Sort    20000           16     0.016000       260632        0
Quick Sort    20000            0     0.000000       361050   177580
Heap Sort     20000            0     0.000000       510441   268162
Merge Sort    40000           16     0.016000       561744        0
Quick Sort    40000            0     0.000000       764368   415046
Heap Sort     40000            0     0.000000      1101368   576765
CLOCKS_PER_SEC on this system = 1000

*/
