/////////////////////i////0////////////////////////////////////////////////////
// tombstones.h, expected interface for CS254 assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <exception>
using namespace std;

#if !defined(__TOMBSTONES_H__)
#define __TOMBSTONES_H__
#include <cstdlib>
template <typename T> class Pointer;
template <typename T> void free(Pointer<T>& obj);

template <typename T>
class Tombstone {
public:
	Tombstone(): cnt(0), pointee(NULL){}
    int cnt;
    T* pointee;
};

template <typename T>
class Pointer {
public:
    Pointer<T>() {
		ts = NULL;
		offset = 0;
	}                               // default constructor

    Pointer<T>(Pointer<T>& p) {
		if (p.ts != NULL) {
			if (p.ts->pointee == NULL) {
				cerr << "Dangling pointer!" << endl;
				terminate();
			}
        	ts = p.ts;
			offset = p.offset;
        	ts->cnt++;
		} else {
			ts = NULL;
		}
    }

    Pointer<T>(T* p) {
		if (p != NULL) {
        	ts = new Tombstone<T>();
        	ts->cnt = 1;
			offset = 0;
        	ts->pointee = p;
		} else {
			ts = NULL;
		}
    }

    ~Pointer<T>() {
		if (ts != NULL){
        	ts->cnt--;
        	if (ts->cnt == 0) {
            	cerr << "Memory leak!" << endl;
				terminate();			
        	}
		}
    }

    T& operator*() const {
        return *(ts->pointee + offset);
    }

    T* operator->() const {
        return ts->pointee;
    }

    Pointer<T>& operator=(const Pointer<T>& p) {
        if (this != &p) {
			if (ts != NULL) {
            	if (ts->cnt == 1) {
                	cerr << "Memory leak!" << endl;
					terminate();
           		}else{
					ts->cnt--;
				}
			}
			if(p.ts != NULL) {
            	ts = p.ts + p.offset;
				offset = 0;
            	ts->cnt++;
			}else {
				ts = NULL;
			}
        }
        return *this;
    }

    // equality comparisons:
    bool operator==(const Pointer<T>& p) const {
        return p.ts == ts;
    }

    bool operator!=(const Pointer<T>& p) const {
        return p.ts != ts;
    }

    bool operator==(const int n) const {
        //  return ts == NULL && n == 0;
        // true iff Pointer is null and int is zero
        if (ts == NULL)
            return n == 0;
        else
            return *(ts->pointee) == n;
    }

    bool operator!=(const int n) const {
        //  return !(ts == NULL && n == 0);
        // false iff Pointer is null and int is zero
        if (ts == NULL)
            return n != 0;
        else
            return *(ts->pointee) != n;
    }

	T& operator[](const int n) {
		return (ts->pointee)[n];
	}

	Pointer<T> operator+(const int n) {
		Pointer<T> np(*this);
		np.offset += n;
		return np;
	}

	Pointer<T> operator-(const int n){
		Pointer<T> np(*this);
		np.offset -= n;
		return np;
	}

	bool operator<(const Pointer<T>& p){
		return ts->pointee + offset < p.ts->pointee + p.offset; 
	}

	bool operator>(const Pointer<T>& p){
		return ts->pointee + offset > p.ts->pointee + p.offset;
	}

	bool operator<=(const Pointer<T>& p){
		return ts->pointee + offset <= p.ts->pointee + p.offset;
	}

	bool operator>=(const Pointer<T>& p){
		return ts->pointee + offset >= p.ts->pointee + p.offset;
	}

    Tombstone<T>* ts;
	int offset;
};
template <class T>
    void free(Pointer<T>& p) {
		if (p.ts != NULL){
        	p.ts->cnt--;
        	if (p.ts->cnt == 0) {
            	delete p.ts->pointee;
            	delete p.ts;            
        	} else {
				cerr << "Dangling pointer!" << endl;
				terminate();
			}
		} else {
			cerr << "Cannot free an uninitialized pointer!" << endl;
			terminate();
		}
    }

template<typename T>
int operator-(const Pointer<T>& p, const Pointer<T>& q){
	return (p.ts->pointee + p.offset) - (q.ts->pointee + q.offset);
}

#endif // __TOMBSTONES_H__
