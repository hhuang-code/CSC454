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
    Tombstone(int c, T* p): cnt(c), pointee(p) {}
    int getCnt() {return cnt;}
    void setCnt(int c) {cnt = c;}
    void increaseCnt() {cnt++;}
    void decreaseCnt() {cnt--;}
    T* getPointee() {return pointee;}
    void setPointee(T* p) {pointee = p;}
private:
    int cnt;
    T* pointee;
};

template <typename T, bool is_checked = true>
class Pointer {
public:
    // default constructor
    Pointer<T, is_checked>() { 
		ts = NULL;
		offset = 0;
		nptr = NULL;
	}

    // copy constructor
    Pointer<T, is_checked>(Pointer<T, is_checked>& p) {
		if (is_checked){
			if (p.getTombstone() != NULL) {
				//if (p.ts->pointee == NULL) {
				//	cerr << "Dangling pointer!" << endl;
				//	terminate();
				//}
        		ts = p.getTombstone();
				offset = p.getOffset();
        		ts->increaseCnt();
			} else {
				ts = NULL;
				offset = p.getOffset();
			}
			nptr = NULL;
		} else {
			nptr = p.getNaivePointer();
			ts = NULL;
			offset = 0;		
		}
    }

    // bootstrapping constructor
    Pointer<T, is_checked>(T* p) {
		if (is_checked) {
			if (p != NULL) {
        		ts = new Tombstone<T>(1, p);
				offset = 0;
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

    // destructor
    ~Pointer<T, is_checked>() {
		if (is_checked) {
			if (ts != NULL){
        		ts->decreaseCnt();
        		if (ts->getCnt() == 0) {
                    if (ts->getPointee() != NULL) { // a valid tombstone, but no Pointer points to it
            		    cerr << "Memory leak!" << endl;
					    terminate();	
                    } else {
                        delete ts;
                    }		
        		}
			}
		}
    }

    T& operator*() const {
		if (ts == NULL && nptr == NULL) {
			cerr << "Cannot deference a null pointer." << endl;
			terminate();
		} else if (ts != NULL) {
        	return *(ts->getPointee() + offset);
		} else {
			return *nptr;
		}
    }

    T* operator->() const {
		if (ts != NULL){
        	return ts->getPointee() + offset;
		} else {
			return nptr;
		}
    }

    Pointer<T, is_checked>& operator=(const Pointer<T, is_checked>& p) {
        if (this != &p) {   // avoid double assignment
			if (is_checked){
				if (ts != NULL) {   // current Pointer points to a tombstone
            		if (ts->getCnt() == 1) {    // Only one Pointer points to this tombstone, and cannot re-assign this Pointer
                		cerr << "Memory leak!" << endl;
						terminate();
           			}else{
						ts->decreaseCnt();  // Other Pointers still point to this tombstone
					}
				}
				if(p.getTombstone() != NULL) {
            		//ts = p.getTombstone() + p.getOffset();
					//offset = 0;
                    ts = p.getTombstone();
                    offset = p.getOffset();
            		ts->increaseCnt();
				} else {
					ts = NULL;
					offset = p.getOffset();
				}
        	} else {
			    nptr = p.getNaivePointer();
		    }
        }   

        return *this;
    }

    // equality comparisons:
    bool operator==(const Pointer<T, is_checked>& p) const {
		if (is_checked) {
        	return (p.getTombstone() == ts && p.getOffset() == offset);
		} else {
			return p.getNaivePointer() == nptr;
		}
    }

    bool operator!=(const Pointer<T, is_checked>& p) const {
		if (is_checked) {
        	return (p.getTombstone() != ts || p.getOffset() != offset);
		} else {
			return p.getNaivePointer() != nptr;
		}
    }

    // true iff Pointer is null and int is zero
    bool operator==(const int n) const {
        if (ts == NULL && nptr == NULL) {
            return true;
		} else if (ts != NULL) {
            return ts->getPointee() == NULL && n == 0;
		} else {
			return false;
		}
    }

    // false iff Pointer is null and int is zero
    bool operator!=(const int n) const {
        if (ts == NULL && nptr == NULL) {
            return false;
		} else if (ts != NULL) {
            return !(ts->getPointee() == NULL && n == 0);
		} else {
			return true;
		}
    }

	T& operator[](const int n) {
		if (ts == NULL && nptr == NULL) {
			cerr << "Cannot deference null pointer." << endl;
			terminate();
		} else if (ts != NULL) {
			return (ts->getPointee())[n];
		} else {
			return nptr[n];
		}
	}

	Pointer<T, is_checked> operator+(const int n) {
		if (is_checked) {
			Pointer<T, is_checked> np(*this);
			np.increaseOffset(n);
			return np;
		} else {
			Pointer<T, is_checked> np(*this);
			np.increaseNaivePointer(n);
			return np;
		}
	}

	Pointer<T, is_checked> operator-(const int n){
		if (is_checked) {
			Pointer<T, is_checked> np(*this);
			np.decreaseOffset(n);
			return np;
		} else {
			Pointer<T, is_checked> np(*this);
			np.decreaseNaivePointer(n);
			return np;
		}
	}

	bool operator<(const Pointer<T, is_checked>& p){
		if (is_checked) {
			if (ts == NULL && p.getTombstone() == NULL){
				return offset < p.getOffset();
			} else if (ts != NULL && p.getTombstone() == NULL) {
				return false;
			} else if (ts == NULL && p.getTombstone() != NULL) {
				return true;
			} else {
				return ts->getPointee() + offset < p.getTombstone()->getPointee() + p.getOffset();
			}	
		} else {
			return nptr < p.getNaivePointer();
		}
	}

	bool operator>(const Pointer<T, is_checked>& p){
		if (is_checked) {
			 if (ts == NULL && p.getTombstone() == NULL){
				 return offset > p.getOffset();
			 } else if (ts != NULL && p.getTombstone() == NULL) {	
				 return true;
			 } else if (ts == NULL && p.getTombstone() != NULL){
				 return false;
			 } else {
			 	return ts->getPointee() + offset > p.getTombstone()->getPointee() + p.getOffset();
			 }
		} else {
			return nptr > p.getNaivePointer();
		}
	}

	bool operator<=(const Pointer<T, is_checked>& p){
		if (is_checked) {
			if (ts == NULL && p.getTombstone() == NULL){
				return offset <= p.getOffset();
			} else if (ts != NULL && p.getTombstone() == NULL) {
				return false;
			} else if (ts == NULL && p.getTombstone() != NULL){
				return true;
			} else {
				return ts->getPointee() + offset <= p.getTombstone()->getPointee() + p.getOffset();
			}
		} else {
			return nptr <= p.getNaivePointer();
		}
	}

	bool operator>=(const Pointer<T, is_checked>& p){
		if (is_checked) {
			if (ts == NULL && p.getTombstone() == NULL){
				return offset >= p.getOffset();
			} else if (ts != NULL && p.getTombstone() == NULL) {   
				return true;
			} else if (ts == NULL && p.getTombstone() != NULL){
				return false;
		   	} else {
				return ts->getPointee() + offset >= p.getTombstone()->getPointee() + p.getOffset();
			}
		} else {
			return nptr >= p.getNaivePointer();
		}
	}

    // setters and getters
    void setTombstone(Tombstone<T>* t) {ts = t;}
    Tombstone<T>* getTombstone() const {return ts;}
    void setOffset(int os) {offset = os;}
    int getOffset() const {return offset;}
    void setNaivePointer(T* p) {nptr = p;}
    T* getNaivePointer() const {return nptr;}
                        
    void increaseOffset(int x) {offset += x;}
    void decreaseOffset(int x) {offset -= x;}

    void increaseNaivePointer(int x) {nptr += x;}
    void decreaseNaivePointer(int x) {nptr -= x;}

private:
    Tombstone<T>* ts;
	int offset;
	T* nptr;    // Naive pointer
};

template <typename T, bool is_checked = true>
    void free(Pointer<T, is_checked>& p) {
		if (is_checked) {
			if (p.getTombstone() != NULL){
        		p.getTombstone()->decreaseCnt();
        		if (p.getTombstone()->getCnt() == 0) {  // no Pointer pointing to the tombstone
            		delete[] p.getTombstone()->getPointee();
                    p.getTombstone()->setPointee(NULL);
            		//delete p.ts;            
        		} else {    // other Pointers point to this tombstone
					cerr << "Dangling pointer!" << endl;
					terminate();
				}
			} else {    // no tombstone
				cerr << "Cannot free an uninitialized pointer!" << endl;
				terminate();
			}
		} else {
			delete[] p.getNaivePointer();
		}
    }

template<typename T, bool is_checked>
int operator-(const Pointer<T, is_checked>& p, const Pointer<T, is_checked>& q){
	if (is_checked) {
		if (p.getTombstone() == NULL && q.getTombstone() == NULL){
			return p.getOffset() - q.getOffset();
		} else if (p.getTombstone() == NULL && q.getTombstone() != NULL) {
			cerr << "Invalid operation for uninitialized pointer." << endl;
			terminate();
		} else if (p.getTombstone() != NULL && q.getTombstone() == NULL) {
			cerr << "Invalid operation for uninitialized pointer." << endl;
			terminate();
		} else {
			return (p.getTombstone()->getPointee() + p.getOffset()) - (q.getTombstone()->getPointee() + q.getOffset());
		}
	} else {
		return p.getNaivePointer() - q.getNaivePointer();
	}
}

#endif // __TOMBSTONES_H__
