/////////////////////////0////////////////////////////////////////////////////
// tombstones.h, expected interface for CS254 assignment 5
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <exception>
using namespace std;

#if !defined(__TOMBSTONES_H__)
#define __TOMBSTONES_H__
#include <cstdlib>
template <class T> class Pointer;
template <class T> void free(Pointer<T>& obj);

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
    int cnt;    // reference count
    T* pointee; // point to the project
};

template <typename T>
class Pointer {
public:
    // default constructor
    Pointer<T>() {
		ts = NULL;
	}

    // copy constructor
    Pointer<T>(Pointer<T>& p) {
		if (p.getTombstone() != NULL) {
			//if (p.ts->pointee == NULL) {
			//	cerr << "Dangling pointer!" << endl;
			//	terminate();
			//}
        	ts = p.getTombstone();
        	ts->increaseCnt();
		} else {
			ts = NULL;
		}
    }

    // bootstrapping constructor
    Pointer<T>(T* p) {
		if (p != NULL) {
        	ts = new Tombstone<T>(1, p);
		} else {
			ts = NULL;
		}
    }

    // destructor
    ~Pointer<T>() {
		if (ts != NULL){
        	ts->decreaseCnt();
        	if (ts->getCnt() == 0) {
                if (ts->getPointee() != NULL) { // a valid tombstone, but no Pointer points to it
            	    cerr << "Memory leak!" << endl;
				    terminate();
                } else {
                    delete ts;  // a dead tombstone, delete it
                }			
        	}
		}
    }

    T& operator*() const {
        return *(ts->getPointee());
    }

    T* operator->() const {
        return ts->getPointee();
    }

    Pointer<T>& operator=(const Pointer<T>& p) {
        if (this != &p) {   // avoid double assignment
			if (ts != NULL) {   // current Pointer points to a tombstone
            	if (ts->getCnt() == 1) {    // Only one Pointer points to this tombstone, and cannot re-assign this Pointer
                	cerr << "Memory leak!" << endl;
					terminate();
           		}else{
					ts->decreaseCnt();
				}
			}
			if(p.getTombstone() != NULL) {
            	ts = p.getTombstone();
            	ts->increaseCnt();
			}else {
				ts = NULL;
			}
        }
        return *this;
    }

    // equality comparisons:
    bool operator==(const Pointer<T>& p) const {
        return p.getTombstone() == ts;
    }

    bool operator!=(const Pointer<T>& p) const {
        return p.getTombstone() != ts;
    }

    // true iff Pointer is null and int is zero
    bool operator==(const int n) const {
        if (ts == NULL) {   // no tombstone
            return true;
        } else {
            return ts->getPointee() == NULL && n == 0;
        }
    }

    // false iff Pointer is null and int is zero
    bool operator!=(const int n) const {
        if (ts == NULL) {   // no tombstone
            return false;
        } else {
            return !(ts->getPointee() == NULL && n == 0);
        }
    }

    // setters and getters
    void setTombstone(Tombstone<T>* t) {ts = t;}
    Tombstone<T>* getTombstone() const {return ts;}

private:
    Tombstone<T>* ts;   // pointing to a tomestone
};

template <typename T>
    void free(Pointer<T>& p) {
		if (p.getTombstone() != NULL){
        	p.getTombstone()->decreaseCnt();
        	if (p.getTombstone()->getCnt() == 0) {  // no Pointer pointing to the tombstone
            	delete p.getTombstone()->getPointee();
                p.getTombstone()->setPointee(NULL);
                //delete p.getTombstone();
        	} else {    // other Pointers point to this tombstone
                if (p.getTombstone()->getPointee() != NULL) {
				    cerr << "Dangling pointer!" << endl;
				    terminate();
                }
			}
		} else {    // no tombstone
			cerr << "Cannot free an uninitialized pointer!" << endl;
			terminate();
		}
    }

#endif // __TOMBSTONES_H__
