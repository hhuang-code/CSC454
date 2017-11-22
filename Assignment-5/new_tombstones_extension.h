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
	Tombstone(): cnt(0), pointee(NULL) {}
	Tombstone(int c, T* p): cnt(c), pointee(p) {}
	int getCnt() {return cnt;}
	void setCnt(int c) {cnt = c;}
	void increaseCnt() {cnt++;}
	void decreaseCnt() {cnt--;}
	T* getPointee() {return pointee;}
	void setPointee(T* p) {pointee = p;}
private:
    int cnt;	// reference count
    T* pointee;	// point to the object
};

template <typename T>
class Pointer {
public:
	// default constructor
    Pointer<T>() {
		ts = NULL;
		offset = 0;
	}
	
	// copy constructor
    Pointer<T>(Pointer<T>& p) {
		if (p.getTombstone() != NULL) {
			//if (p.getTombstone()->getPointee() == NULL) {
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
    }
	
	// bootstrapping constructor
    Pointer<T>(T* p) {
		if (p != NULL) {
        	ts = new Tombstone<T>(1, p);
			offset = 0;
		} else {
			ts = NULL;
			offset = 0;
		}
    }

	// destructor 
    ~Pointer<T>() {
		if (ts != NULL){
        	ts->decreaseCnt();
        	if (ts->getCnt() == 0) {
				if (ts->getPointee() != NULL) {	// a valid tombstone, but no Pointer points to it
            		cerr << "Memory leak!" << endl;
					terminate();			
				} else {
					delete ts;	// a dead tombstone, delete it
				}
        	}
		}
    }

    T& operator*() const {
        return *(ts->getPointee() + offset);
    }

    T* operator->() const {
        return ts->getPointee() + offset;
    }

    Pointer<T>& operator=(const Pointer<T>& p) {
        if (this != &p) {	// avoid double assignment
			if (ts != NULL) {	// current Pointer points to a tombstone
            	if (ts->getCnt() == 1) {	// Only one Pointer points to this tombstone, and cannot re-assign this Pointer
                	cerr << "Memory leak!" << endl;
					terminate();
           		}else{
					ts->decreaseCnt();	// Other Pointers still point to this tombstone
				}
			}
			if(p.getTombstone() != NULL) {
            	ts = p.getTombstone() + p.getOffset();
				offset = 0;
            	ts->increaseCnt();
			}else {
				ts = NULL;
				offset = p.getOffset();
			}
        }
        return *this;
    }

    // equality comparisons:
    bool operator==(const Pointer<T>& p) const {
        return (p.getTombstone() == ts && p.getOffset() == offset);
    }

    bool operator!=(const Pointer<T>& p) const {
        return (p.getTombstone() != ts || p.getOffset() != offset);
    }

	// true iff Pointer is null and int is zero
    bool operator==(const int n) const {	
		if (ts == NULL) {	// no tombstone
			return true;
		} else {
			return ts->getPointee() == NULL && n == 0;
		}
    }

	// false iff Pointer is null and int is zero
    bool operator!=(const int n) const {
		if (ts == NULL) {	// no tombstone
			return false;
		} else {
			return !(ts->getPointee() == NULL && n == 0);
		}
    }

	T& operator[](const int n) {
		return (ts->getPointee())[n];
	}

	Pointer<T> operator+(const int n) {
		Pointer<T> np(*this);
		np.increaseOffset(n);
		return np;
	}

	Pointer<T> operator-(const int n){
		Pointer<T> np(*this);
		np.decreaseOffset(n);
		return np;
	}

	bool operator<(const Pointer<T>& p){
		return ts->getPointee() + offset < p.getTombstone()->getPointee() + p.getOffset(); 
	}

	bool operator>(const Pointer<T>& p){
		return ts->getPointee() + offset > p.getTombstone()->getPointee() + p.getOffset();
	}

	bool operator<=(const Pointer<T>& p){
		return ts->getPointee() + offset <= p.getTombstone()->getPointee() + p.getOffset();
	}

	bool operator>=(const Pointer<T>& p){
		return ts->getPointee() + offset >= p.getTombstone()->getPointee() + p.getOffset();
	}
	
	// setters and getters
	void setTombstone(Tombstone<T>* t) {ts = t;}
	Tombstone<T>* getTombstone() const {return ts;}
	void setOffset(int os) {offset = os;}
	int getOffset() const {return offset;}
	
	void increaseOffset(int x) {offset += x;}
	void decreaseOffset(int x) {offset -= x;}

private:
    Tombstone<T>* ts;	// pointing to a tombstone
	int offset;	// used for array Pointer
};

template <typename T>
void free(Pointer<T>& p) {
	if (p.getTombstone() != NULL){
        p.getTombstone()->decreaseCnt();
        if (p.getTombstone()->getCnt() == 0) {	// no Pointer pointing to the tombstone
            delete p.getTombstone()->getPointee();
            //delete p.getTombstone();            
        } else {	// other Pointers point to this tombstone
			if (p.getTombstone()->getPointee() != NULL) {
				cerr << "Dangling pointer!" << endl;
				terminate();
			}
		}
	} else {	// no tombstone
		cerr << "Cannot free an uninitialized pointer!" << endl;
		terminate();
	}
}

template<typename T>
int operator-(const Pointer<T>& p, const Pointer<T>& q){
	return (p.getTombstone()->getPointee() + p.getOffset()) - (q.getTombstone()->getPointee() + q.getOffset());
}

#endif // __TOMBSTONES_H__
