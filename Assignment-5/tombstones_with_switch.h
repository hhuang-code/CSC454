/////////////////////i////0////////////////////////////////////////////////////
// tombstones.h, expected interface for CS254 assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <exception>
using namespace std;

#if !defined(__TOMBSTONES_H__)
#define __TOMBSTONES_H__
#include <cstdlib>
template <typename T, bool is_checked> class Pointer;
template <typename T> void free(Pointer<T, true>& obj);

template <typename T>
class Tombstone {
public:
	Tombstone(): cnt(0), pointee(NULL){}
    int cnt;
    T* pointee;
};

template <typename T, bool is_checked = true>
class Pointer {
public:
    Pointer<T, is_checked>() { 
		ts = NULL;
		offset = 0;
		nptr = NULL;
	}                               // default constructor

    Pointer<T, is_checked>(Pointer<T, is_checked>& p) {
		if (is_checked){
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
				offset = p.offset;
			}
			nptr = NULL;
		} else {
			nptr = p.nptr;
			ts = NULL;
			offset = 0;		
		}
    }

    Pointer<T, is_checked>(T* p) {
		if (is_checked) {
			if (p != NULL) {
        		ts = new Tombstone<T>();
        		ts->cnt = 1;
				offset = 0;
        		ts->pointee = p;
			} else {
				ts = NULL;
				offset = 0;
			}
			nptr = NULL;
		} else {
			nptr = p;
			ts = NULL;
			offset = 0;
		}
    }

    ~Pointer<T, is_checked>() {
		if (is_checked) {
			if (ts != NULL){
        		ts->cnt--;
        		if (ts->cnt == 0) {
            		cerr << "Memory leak!" << endl;
					terminate();			
        		}
			}
		}
    }

    T& operator*() const {
		if (ts == NULL && nptr == NULL) {
			cerr << "Cannot deference null pointer." << endl;
			terminate();
		} else if (ts != NULL) {
        	return *(ts->pointee + offset);
		} else {
			return *nptr;
		}
    }

    T* operator->() const {
		if (ts != NULL){
        	return ts->pointee;
		} else {
			return nptr;
		}
    }

    Pointer<T, is_checked>& operator=(const Pointer<T, is_checked>& p) {
        if (this != &p) {
			if (is_checked){
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
				} else {
					ts = NULL;
					offset = p.offset;
				}
        	}
		} else {
			nptr = p.nptr;
		}

        return *this;
    }

    // equality comparisons:
    bool operator==(const Pointer<T, is_checked>& p) const {
		if (is_checked) {
        	return p.ts == ts;
		} else {
			return p.nptr == nptr;
		}
    }

    bool operator!=(const Pointer<T, is_checked>& p) const {
		if (is_checked) {
        	return p.ts != ts;
		} else {
			return p.nptr != p.nptr;
		}
    }

    bool operator==(const int n) const {
        //  return ts == NULL && n == 0;
        // true iff Pointer is null and int is zero
        if (ts == NULL && nptr == NULL) {
            return n == 0;
		} else if (ts != NULL) {
            return *(ts->pointee) == n;
		} else {
			return *nptr == n;
		}
    }

    bool operator!=(const int n) const {
        //  return !(ts == NULL && n == 0);
        // false iff Pointer is null and int is zero
        if (ts == NULL && nptr == NULL) {
            return n != 0;
		} else if (ts != NULL) {
            return *(ts->pointee) != n;
		} else {
			return *nptr != n;
		}
    }

	T& operator[](const int n) {
		if (ts == NULL && nptr == NULL) {
			cerr << "Cannot deference null pointer." << endl;
			terminate();
		} else if (ts != NULL) {
			return (ts->pointee)[n];
		} else {
			return nptr[n];
		}
	}

	Pointer<T, is_checked> operator+(const int n) {
		if (is_checked) {
			Pointer<T, is_checked> np(*this);
			np.offset += n;
			return np;
		} else {
			Pointer<T, is_checked> np(*this);
			np.nptr += n;
			return np;
		}
	}

	Pointer<T, is_checked> operator-(const int n){
		if (is_checked) {
			Pointer<T, is_checked> np(*this);
			np.offset -= n;
			return np;
		} else {
			Pointer<T, is_checked> np(*this);
			np.nptr -= n;
			return np;
		}
	}

	bool operator<(const Pointer<T, is_checked>& p){
		if (is_checked) {
			if (ts == NULL && p.ts == NULL){
				return offset < p.offset;
			} else if (ts != NULL && p.ts == NULL) {
				return false;
			} else if (ts == NULL && p.ts != NULL) {
				return true;
			} else {
				return ts->pointee + offset < p.ts->pointee + p.offset;
			}	
		} else {
			return nptr < p.nptr;
		}
	}

	bool operator>(const Pointer<T, is_checked>& p){
		if (is_checked) {
			 if (ts == NULL && p.ts == NULL){
				 return offset > p.offset;
			 } else if (ts != NULL && p.ts == NULL) {	
				 return true;
			 } else if (ts == NULL && p.ts != NULL){
				 return false;
			 } else {
			 	return ts->pointee + offset > p.ts->pointee + p.offset;
			 }
		} else {
			return nptr > p.nptr;
		}
	}

	bool operator<=(const Pointer<T, is_checked>& p){
		if (is_checked) {
			if (ts == NULL && p.ts == NULL){
				return offset <= p.offset;
			} else if (ts != NULL && p.ts == NULL) {
				return false;
			} else if (ts == NULL && p.ts != NULL){
				return true;
			} else {
				return ts->pointee + offset <= p.ts->pointee + p.offset;
			}
		} else {
			return nptr <= p.nptr;
		}
	}

	bool operator>=(const Pointer<T, is_checked>& p){
		if (is_checked) {
			if (ts == NULL && p.ts == NULL){
				return offset >= p.offset;
			} else if (ts != NULL && p.ts == NULL) {   
				return true;
			} else if (ts == NULL && p.ts != NULL){
				return false;
		   	} else {
				return ts->pointee + offset >= p.ts->pointee + p.offset;
			}
		} else {
			return nptr >= p.nptr;
		}
	}

    Tombstone<T>* ts;
	int offset;
	T* nptr;
};

template <typename T, bool is_checked = true>
    void free(Pointer<T, is_checked>& p) {
		if (is_checked) {
			if (p.ts != NULL){
        		p.ts->cnt--;
        		if (p.ts->cnt == 0) {
            		delete[] p.ts->pointee;
            		delete p.ts;            
        		} else {
					cerr << "Dangling pointer!" << endl;
					terminate();
				}
			} else {
				cerr << "Cannot free an uninitialized pointer!" << endl;
				terminate();
			}
		} else {
			delete[] p.nptr;
		}
    }

template<typename T, bool is_checked>
int operator-(const Pointer<T, is_checked>& p, const Pointer<T, is_checked>& q){
	if (is_checked) {
		if (p.ts == NULL && q.ts == NULL){
			return p.offset - q.offset;
		} else if (p.ts == NULL && q.ts != NULL) {
			cerr << "Invalid operation for uninitialized pointer." << endl;
			terminate();
		} else if (p.ts != NULL && q.ts == NULL) {
			cerr << "Invalid operation for uninitialized pointer." << endl;
			terminate();
		} else {
			return (p.ts->pointee + p.offset) - (q.ts->pointee + q.offset);
		}
	} else {
		return p.nptr - q.nptr;
	}
}

#endif // __TOMBSTONES_H__
