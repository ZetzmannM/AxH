#ifndef __H_DATA
#define __H_DATA

#include <exception>
#include <string>
#include <sstream>

#include "dtype.h"

#include "ErrorHandling.h"

#include <math.h>
#include "inc_settings.h"

template<typename T> using owner = T;    //no ownership assumed otherwise
template<typename T> using nullable = T; //non_null assumed otherwise !!

///<summary>
/// The man_ptr, man_null_ptr and man_arr_ptr are designed for passing pointers of which there should only be one occurance.
/// If nothing extracts the pointer from the object by calling get(), the object is automatically deleted when the deconstructor is called.
/// The copyFrom constructor and the copyFrom assignment operator both move the pointer from the incoming object to the recieving.
///</summary>
template<typename T> struct pass_ptr {
protected:
	owner<T*> ptr = nullptr;

	pass_ptr() {}

public:
	///<summary>
	///Creates a managed pointer object
	///</summary>
	pass_ptr(owner<T*>& ptra);

	///<summary>
	///Creates a managed pointer object
	///</summary>
	pass_ptr(owner<T*>&& ptra);

	///<summary>
	///Creates a managed pointer object
	///</summary>
	pass_ptr(pass_ptr<T>& ref);
	pass_ptr(pass_ptr<T>&& ref) noexcept;
	virtual ~pass_ptr();

	///<summary>
	///Returns the encapsuled pointer and invalidates the managed pointer object
	///</summary>
	virtual owner<T*> get();

	T* operator->();
	virtual void operator=(pass_ptr<T>& ref);

	///<summary>
	///deletes the encapsuled object and invalidates the pointer
	///</summary>
	virtual void discard();

	///<summary>
	///Returns whether the pointer object is currently holding a valid pointer
	///</summary>
	bool valid() const;

	///<summary>
	/// Returns a refence to the object. This won't invalidate the managed pointer object
	///</summary>
	T& getReference();

	///<summary>
	/// Returns a refence to the object. This won't invalidate the managed pointer object
	///</summary>
	const T& getReference() const;

	///<summary>
	/// Returns a const copyFrom of the encapsuled pointer. This won't invalidate the managed pointer object
	///</summary>
	const T* getPtrCopy();

protected:
	///<summary>
	///Invalidates the pointer object without deleting it.
	///</summary>
	inline void _inv();
};
///<summary>
/// The man_ptr, man_null_ptr and man_arr_ptr are designed for passing pointers of which there should only be one occurance.
/// If nothing extracts the pointer from the object by calling get(), the object is automatically deleted when the deconstructor is called.
/// The copyFrom constructor and the copyFrom assignment operator both move the pointer from the incoming object to the recieving.
///</summary>
template<typename T> struct pass_null_ptr : pass_ptr<T> {
	pass_null_ptr(owner<T*>& ptra);
	pass_null_ptr(owner<T*>&& ptra);
	pass_null_ptr(pass_ptr<T>& ref);
	pass_null_ptr(pass_null_ptr<T>& ref);
	pass_null_ptr(pass_null_ptr<T>&& ref);
	pass_null_ptr();
	virtual ~pass_null_ptr() {}

	///<summary>
	///returns the object as a man_ptr
	///</summary>
	pass_ptr<T> getDataPointer();

	virtual void operator=(pass_ptr<T>& ref) override;
	virtual owner<T*> get() override;
	virtual void discard() override;
};
///<summary>
/// The man_ptr, man_null_ptr and man_arr_ptr are designed for passing pointers of which there should only be one occurance.
/// If nothing extracts the pointer from the object by calling get(), the object is automatically deleted when the deconstructor is called.
/// The copyFrom constructor and the copyFrom assignment operator both move the pointer from the incoming object to the recieving.
///</summary>
template<typename T> struct pass_arr_ptr : pass_null_ptr<T> {
private:
	uint32 sze = 0;

public:
	pass_arr_ptr(owner<T*>& ptra, int size);
	pass_arr_ptr(owner<T*>&& ptra, int size);
	pass_arr_ptr(pass_arr_ptr<T>& ref);
	pass_arr_ptr(pass_ptr<T>& ref, int size = 1);
	pass_arr_ptr(int size);
	pass_arr_ptr();
	virtual ~pass_arr_ptr() override;

	virtual void operator=(pass_arr_ptr<T>& ref);

	///<summary>
	///Returns the object at the index of the array
	///</summary>
	T& operator[](int index);
	const T& operator[](int index) const;

	virtual void discard() override;

	uint32 size();

};

///<summary>
/// The wrap_ptr object are designed to wrap pointers to dynamically allocated data objects in a way that allows them to be treated like statically allocated data.
/// This means, that the deconstructor of the wrap_ptr deletes the object, the copyFrom constructor deep copies the object, the assignment operator automatically copies the data from the passed object into the current (deep copyFrom).
/// Move assignement operator and move (copyFrom) constructor are defined as well.
///</summary>
template<typename T> struct wrap_ptr {
protected:
	owner<T*> ptr = 0;

public:
	///<summary>
	///Creates a managed pointer object
	///</summary>
	wrap_ptr(owner<T*>&& ptra);

	///<summary>
	/// Creates a null wrap pointer.
	///</summary>
	wrap_ptr();

	///<summary>
	///Creates a managed pointer object by copying the passed object (deep copyFrom)
	///</summary>
	wrap_ptr(const wrap_ptr<T>& ref);

	///<summary>
	///Creates a managed pointer object by copying the passed object (moves the pointer from the passed object into the current)
	///</summary>
	wrap_ptr(wrap_ptr<T>&& ref);
	wrap_ptr(pass_ptr<T>& ref);
	wrap_ptr(pass_ptr<T>&& ref);

	///<summary>
	/// Deletes the object
	///</summary>
	~wrap_ptr();


	T* operator->();

	///<summary>
	///Sets the wrap pointer by copying the passed object (deep copyFrom)
	///</summary>
	void operator=(const wrap_ptr<T>& ref);

	///<summary>
	///Sets the wrap pointer by copying the passed object (moves the pointer from the passed object into the current)
	///</summary>
	void operator=(wrap_ptr<T>&& ref) noexcept;

	///<summary>
	///Sets the wrap pointer by copying the passed object (deep copyFrom)
	///</summary>
	void operator=(pass_ptr<T>& ref);

	///<summary>
	///Sets the wrap pointer by copying the passed object (moves the pointer from the passed object into the current)
	///</summary>
	void operator=(pass_ptr<T>&& ref);

	///<summary>
	///Returns whether the pointer object is currently holding a valid pointer
	///</summary>
	bool valid() const;
	///<summary>
	///deletes the encapsuled object and invalidates the pointer
	///</summary>
	void discard();

	const T& getReference() const;
	T& getReference();

	///<summary>
	///Extracts the pointer and marks the object as invalid
	///</summary>
	pass_null_ptr<T> extract();

	T* _cpy() const {
		return this->ptr;
	}
};
///<summary>
/// The wrap_ptr object are designed to wrap pointers to dynamically allocated data objects in a way that allows them to be treated like statically allocated data.
/// This means, that the deconstructor of the wrap_ptr deletes the object, the copyFrom constructor deep copies the object, the assignment operator automatically copies the data from the passed object into the current (deep copyFrom).
/// Move assignement operator and move (copyFrom) constructor are defined as well.
///</summary>
template<typename T> struct wrap_arr_ptr {
protected:
	owner<T*> ptr = 0;
	size_t _size;
public:
	///<summary>
	///Creates a wrapped pointer. Uses the passed pointer
	///</summary>
	///<param name='size'>Size of the array</param>
	wrap_arr_ptr(owner<T*>&& ptra, size_t size);

	///<summary>
	///Creates a wrapped pointer and an array
	///</summary>
	///<param name='size'>Size of the array</param>
	wrap_arr_ptr(size_t size);

	///<summary>
	/// Creates a null wrap pointer.
	///</summary>
	wrap_arr_ptr();

	///<summary>
	///Creates a managed pointer object by copying the passed object (deep copyFrom)
	///</summary>
	wrap_arr_ptr(const wrap_arr_ptr<T>& ref);

	///<summary>
	///Creates a managed pointer object by copying the passed object (moves the pointer from the passed object into the current)
	///</summary>
	wrap_arr_ptr(wrap_arr_ptr<T>&& ref);

	///<summary>
	/// Deletes the object
	///</summary>
	~wrap_arr_ptr();

	///<summary>
	///Sets the wrap pointer by copying the passed object (deep copyFrom)
	///</summary>
	void operator=(pass_arr_ptr<T>& ref);

	///<summary>
	///Sets the wrap pointer by copying the passed object (moves the pointer from the passed object into the current)
	///</summary>
	void operator=(pass_arr_ptr<T>&& ref);

	void operator=(const wrap_arr_ptr<T>& ref);

	void operator=(wrap_arr_ptr<T>&& ref);

	T& operator[](size_t ind);
	const T& operator[](size_t ind) const;

	///<summary>
	///Returns whether the pointer object is currently holding a valid pointer
	///</summary>
	bool valid();

	///<summary>
	/// Returns the size of the array
	///</summary>
	size_t size();

	///<summary>
	///deletes the encapsuled object and invalidates the pointer
	///</summary>
	void discard();

	///<summary>
	///Extracts the pointer and marks the object as invalid
	///Transmits ownership
	///</summary>
	pass_ptr<T> extract();

	///<summary>
	///This method returns a const ptr to the array
	///THE APPLICATION MUST ENSURE THAT THIS POINTER IS NOT DELETED, AS OWNERSHIP IS NOT TRANSMITTED!
	///</summary>
	const T* data() const;

	///<summary>
	///This method returns a const ptr to the array
	///THE APPLICATION MUST ENSURE THAT THIS POINTER IS NOT DELETED, AS OWNERSHIP IS NOT TRANSMITTED!
	///</summary>
	T* data();

};

template<typename T> struct DefaultDuplicator {
	T* operator()(const T& in) {
		return new T(in);
	}
};
template<typename T, class D = DefaultDuplicator<T>> struct ptr_vector {
private:
	std::vector<owner<T*>> cnt;
	void _delPtr() {
		auto it = cnt.begin();
		while (it != cnt.end()) {
#ifdef __DEBUG
			std::stringstream ss;
			ss << PTRSTR(*it);
			ss << std::string(", ") << (typeid(T).name());
			PRINT(ss.str(), CHANNEL_DECONST_DEBUG);
#endif
			delete* it;
			*it = 0;
			it++;
		}
	}
public:

	ptr_vector(std::initializer_list<pass_ptr<T>>&& init) {
		auto it = init.begin();
		while (it != init.end()) {
			this->push_back(*it);
			it++;
		}
	}
	ptr_vector() {}
	ptr_vector(const ptr_vector& in) {
		this->operator=(in);
	}
	~ptr_vector() {
#ifdef __DEBUG
		PRINT(std::string(typeid(T).name()) + " " + PTRSTR(this), CHANNEL_DECONST_DEBUG);
#endif
		_delPtr();
	}

	void operator=(const ptr_vector& in) {
		_delPtr();
		D d;
		for (uint32 i = 0; i < in.cnt.size(); i++) {
			cnt.push_back(d(*in.cnt.at(i)));
		}
	}

	void operator=(ptr_vector&& in) {
		_delPtr();
		this->cnt = in.cnt;
	}

	T& operator[](uint32 ind) {
		return *cnt[ind];
	}

	T*& operator()(uint32 ind) {
		return cnt[ind];
	}

	void push_back(pass_ptr<T>& ptr) {
		this->cnt.push_back(ptr.get());
	}
	void push_back(pass_ptr<T>&& ptr) {
		this->cnt.push_back(ptr.get());
	}

	T& at(const uint32& ref) {
		return this->operator[](ref);
	}
	T* access(const uint32 ref) {
		return cnt.at(ref);
	}
	const T* access(const uint32 ref) const {
		return cnt.at(ref);
	}
	void eraseAll() {
		_delPtr();
		this->cnt.clear();
	}
	void erase(uint32 ind) {
		T* ptr = this->cnt.at(ind);
		delete ptr;
		cnt.erase(begin() + ind);
	}

	void resize(uint32 size) {
		this->cnt.resize(size);
	}

	auto begin() {
		return cnt.begin();
	}
	auto end() {
		return cnt.end();
	}
	auto begin() const {
		return cnt.begin();
	}
	auto end() const {
		return cnt.end();
	}
	uint64 size() const {
		return cnt.size();
	}
};

template<typename T> void pass_ptr<T>::_inv() {
	this->ptr = 0;
}
template<typename T> const T* pass_ptr<T>::getPtrCopy() {
	return ptr;
}
template<typename T> const T& pass_ptr<T>::getReference() const {
	if (ptr) {
		return *ptr;
	}
	PRINT_ERR("(NullPointer)", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
}
template<typename T> T& pass_ptr<T>::getReference() {
	if (ptr) {
		return *ptr;
	}
	PRINT_ERR("(NullPointer)", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
}
template<typename T> bool pass_ptr<T>::valid() const {
	return ptr;
}
template<typename T> void pass_ptr<T>::discard() {
	if (ptr) {
		delete ptr;
		_inv();
	}
}
template<typename T> void pass_ptr<T>::operator=(pass_ptr<T>& ref) {
	if (ptr) {
		delete ptr;
	}
	ptr = ref.get();
}
template<typename T> T* pass_ptr<T>::operator->() {
	return ptr;
}
template<typename T> T* pass_ptr<T>::get() {
	if (ptr) {
		T* temp = ptr;
		this->_inv();
		return temp;
	}
	else {
		PRINT_ERR("Invalid Pointer Operation! (NullPointer)", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
	return NULL;
}
template<typename T> pass_ptr<T>::~pass_ptr() {
	if (ptr) {
		delete ptr;
	}
}
template<typename T> pass_ptr<T>::pass_ptr(pass_ptr<T>& ref) {
	ptr = ref.get();
}
template<typename T> pass_ptr<T>::pass_ptr(pass_ptr<T>&& ref) noexcept {
	ptr = ref.get();
}
template<typename T> pass_ptr<T>::pass_ptr(owner<T*>&& ptra) {
	if (ptra) {
		this->ptr = ptra;
	}
	else {
		PRINT_ERR("Nullpointers are not allowd!", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
}
template<typename T> pass_ptr<T>::pass_ptr(owner<T*>& ptra) {
	if (ptra) {
		this->ptr = ptra;
		ptra = 0;
	}
	else {
		PRINT_ERR("Nullpointers are not allowd!", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
}
template<typename T> pass_null_ptr<T>::pass_null_ptr(owner<T*>& ptra) {
	this->ptr = ptra;
	ptra = nullptr;
}
template<typename T> pass_null_ptr<T>::pass_null_ptr(owner<T*>&& ptra) {
	this->ptr = ptra;
}
template<typename T> pass_null_ptr<T>::pass_null_ptr(pass_ptr<T>& ref) {
	this->ptr = ref.get();
}
template<typename T> pass_null_ptr<T>::pass_null_ptr(pass_null_ptr<T>& ref) {
	this->ptr = ref.get();
}
template<typename T> pass_null_ptr<T>::pass_null_ptr(pass_null_ptr<T>&& ref) {
	this->ptr = ref.get();
}
template<typename T> pass_null_ptr<T>::pass_null_ptr() {
	this->ptr = nullptr;
}

template<typename T> pass_ptr<T> pass_null_ptr<T>::getDataPointer() {
	return pass_ptr<T>(get());
}
template<typename T> void pass_null_ptr<T>::operator=(pass_ptr<T>& ref) {
	discard();
	this->ptr = ref.get();
}
template<typename T> T* pass_null_ptr<T>::get() {
	T* temp = this->ptr;
	this->_inv();
	return temp;
}
template<typename T> void pass_null_ptr<T>::discard() {
	if (this->ptr) {
		delete this->ptr;
		this->_inv();
	}
}
template<typename T> pass_arr_ptr<T>::pass_arr_ptr() : pass_ptr<T>(0) {}
template<typename T> pass_arr_ptr<T>::pass_arr_ptr(owner<T*>& ptra, int size) : pass_ptr<T>(ptra) {
	this->sze = size;
}
template<typename T> pass_arr_ptr<T>::pass_arr_ptr(owner<T*>&& ptra, int size) : pass_ptr<T>(ptra) {
	this->sze = size;
}
template<typename T> pass_arr_ptr<T>::pass_arr_ptr(pass_arr_ptr<T>& ref) {
	this->operator=(ref);
}
template<typename T> pass_arr_ptr<T>::pass_arr_ptr(pass_ptr<T>& ref, int size) : pass_ptr<T>(ref) {
	this->sze = size;
}
template<typename T> pass_arr_ptr<T>::pass_arr_ptr(int size) : pass_arr_ptr<T>(new T[size], size) { }
template<typename T> void pass_arr_ptr<T>::operator=(pass_arr_ptr<T>& ref) {
	discard();
	this->ptr = ref.get();

	this->sze = ref.size();
}
template<typename T> T& pass_arr_ptr<T>::operator[](int index) {
	if (this->ptr && index < this->sze) {
		return this->ptr[index];
	}
	else {
		PRINT_ERR(std::string("Invalid Access through operator[]: Invalid Size [param:") + std::to_string(index) + std::string(",size:") + std::to_string(size) + std::string("] or nonexisting Pointer"), PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
}
template<typename T> const T& pass_arr_ptr<T>::operator[](int index) const {
	if (this->ptr && index < this->sze) {
		return this->ptr[index];
	}
	else {
		PRINT_ERR(std::string("Invalid Access through operator[]: Invalid Size [param:") + std::to_string(index) + std::string(",size:") + std::to_string(size) + std::string("] or nonexisting Pointer"), PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
}
template<typename T> pass_arr_ptr<T>::~pass_arr_ptr() {
	if (this->ptr) {
		delete[] this->ptr;
	}
}
template<typename T> void pass_arr_ptr<T>::discard() {
	pass_null_ptr<T>::discard();
	this->sze = 0;
}
template<typename T>
uint32 pass_arr_ptr<T>::size() {
	return this->sze;
}
template<typename T> wrap_ptr<T>::wrap_ptr(owner<T*>&& ptra) {
	this->ptr = ptra;
}
template<typename T> wrap_ptr<T>::wrap_ptr() {
	this->ptr = 0;
}
template<typename T> wrap_ptr<T>::wrap_ptr(const wrap_ptr<T>& ref) {
	if (valid()) {
		this->ptr->operator=(*ref.ptr);
	}
	else {
		this->ptr = new T(*ref.ptr);
	}
}
template<typename T> wrap_ptr<T>::wrap_ptr(wrap_ptr<T>&& ref) {
	discard();
	this->ptr = ref.extract().get();
}
template<typename T> wrap_ptr<T>::wrap_ptr(pass_ptr<T>& ref) {
	this->operator=(ref);
}
template<typename T> wrap_ptr<T>::wrap_ptr(pass_ptr<T>&& ref) {
	this->operator=(ref);
}
template<typename T> wrap_ptr<T>::~wrap_ptr() {
	if (valid()) {
		delete ptr;
	}
}
template<typename T> T* wrap_ptr<T>::operator->() {
	return ptr;
}
template<typename T> void wrap_ptr<T>::operator=(const wrap_ptr<T>& ref) {
	discard();
	if (ref.valid()) {
		this->ptr = new T(*ref.ptr);
	}
	else {
		this->ptr = nullptr;
	}
}
template<typename T> void wrap_ptr<T>::operator=(wrap_ptr<T>&& ref) noexcept {
	discard();
	if (ref.valid()) {
		this->ptr = ref.extract().get();
	}
	else {
		this->ptr = nullptr;
	}
}
template<typename T> bool wrap_ptr<T>::valid() const {
	return ptr;
}
template<typename T> void wrap_ptr<T>::discard() {
	if (ptr) {
		delete ptr;
		ptr = 0;
	}
}
template<typename T> pass_null_ptr<T> wrap_ptr<T>::extract() {
	T* copy = ptr;
	ptr = nullptr;
	return pass_null_ptr<T>(copy);
}
template<typename T> void wrap_ptr<T>::operator=(pass_ptr<T>& ref) {
	discard();
	this->ptr = ref.get();
}
template<typename T> void wrap_ptr<T>::operator=(pass_ptr<T>&& ref) {
	discard();
	this->ptr = ref.get();
}
template<typename T> const T& wrap_ptr<T>::getReference() const {
	return *ptr;
}
template<typename T> T& wrap_ptr<T>::getReference() {
	return *ptr;
}

template<typename T> wrap_arr_ptr<T>::wrap_arr_ptr(owner<T*>&& ptra, size_t size) {
	this->ptr = ptra;
	this->_size = size;
}
template<typename T> wrap_arr_ptr<T>::wrap_arr_ptr(size_t size) {
	this->ptr = new T[size];
	this->_size = size;
}
template<typename T> wrap_arr_ptr<T>::wrap_arr_ptr() {
	this->ptr = nullptr;
	this->_size = 0;
}
template<typename T> wrap_arr_ptr<T>::wrap_arr_ptr(const wrap_arr_ptr<T>& ref) {
	this->_size = ref._size;
	this->ptr = new T[this->_size];
	for (size_t i = 0; i < _size; i++) {
		this->ptr[i] = ref[i];
	}
}
template<typename T> wrap_arr_ptr<T>::wrap_arr_ptr(wrap_arr_ptr<T>&& ref) {
	this->ptr = ref.extract().get();
	this->_size = ref._size;
}
template<typename T> wrap_arr_ptr<T>::~wrap_arr_ptr() {
	discard();
}
template<typename T> void wrap_arr_ptr<T>::operator=(pass_arr_ptr<T>& ref) {
	discard();
	this->_size = ref._size;
	this->ptr = ref.get();
}
template<typename T> void wrap_arr_ptr<T>::operator=(pass_arr_ptr<T>&& ref) {
	discard();
	this->_size = ref._size;
	this->ptr = ref.get();
}
template<typename T> void wrap_arr_ptr<T>::operator=(const wrap_arr_ptr<T>& ref) {
	discard();
	this->_size = ref._size;
	this->ptr = ref.extract().get(); // @suppress("Invalid arguments")
}
template<typename T> void wrap_arr_ptr<T>::operator=(wrap_arr_ptr<T>&& ref) {
	discard();
	this->_size = ref._size;
	this->ptr = ref.extract().get();
}

template<typename T> bool wrap_arr_ptr<T>::valid() {
	return ptr;
}
template<typename T> T* wrap_arr_ptr<T>::data() {
	return this->ptr;
}
template<typename T> const T* wrap_arr_ptr<T>::data() const {
	return this->ptr;
}
template<typename T> T& wrap_arr_ptr<T>::operator[](size_t ind) {
	if (valid() && ind < _size) {
		return ptr[ind];
	}
	else {
		PRINT_ERR("Invalid Array Access!", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
}
template<typename T> const T& wrap_arr_ptr<T>::operator[](size_t ind) const {
	if (this->valid() && ind < _size) {
		return ptr[ind];
	}
	else {
		PRINT_ERR("Invalid Array Access!", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG);
	}
}
template<typename T> size_t wrap_arr_ptr<T>::size() {
	return _size;
}
template<typename T> void wrap_arr_ptr<T>::discard() {
	if (valid()) {
		delete[] ptr;
		_size = 0;
	}
}
template<typename T> pass_ptr<T> wrap_arr_ptr<T>::extract() {
	this->_size = 0;
	T* copy = ptr;
	ptr = nullptr;
	return pass_ptr<T>(copy);
}
#endif
