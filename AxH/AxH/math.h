#pragma once

#include <array>
#include <tuple>

#include <type_traits>
#include <utility>

#include "errhndl.h"
#include "dtypes.h"
#include "utils.h"

namespace math {

	template <bool...> struct bool_pack;
	template <bool... v>
	using all_true = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>;

	template<size_t... A>
	struct descriptor_size;
	template<>
	struct descriptor_size<> {
		static constexpr size_t value = 1;
	};
	template<size_t A, size_t... S>
	struct descriptor_size<A, S...> {
		static constexpr size_t value = A * descriptor_size<S...>::value;
	};
	
	template<size_t A, size_t... S>
	/// <summary>
	/// Class for accessing elements from multidimensional arrays saved in a on dimensional array.
	/// </summary>
	class descriptor {
		static constexpr size_t dim = sizeof...(S) + 1;
		size_t offset = 0;
	public:
		std::array<size_t, dim> extends;
		std::array<size_t, dim> strides;
	

		descriptor(size_t offset, std::array<size_t, dim>&& strides){
			this->offset = offset;
			this->extends = { A, size_t(S)... };
			this->strides = std::forward<std::array<size_t, dim>>(strides);
		}

		template<typename... S,
				typename = std::enable_if<all_true<std::is_convertible<S, size_t>::value...>::value>::type>
		size_t operator()(S... t) const {
			static_assert(sizeof...(t) == dim);

			size_t ind[dim]{ size_t(t)... };
			size_t index = offset;
			for (size_t i = 0; i < dim; ++i) {
				if (ind[i] >= extends[i]) {
					PRINT_ERR("subscript out of bounds", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG)
				}
				index += strides[i] * ind[i];
			}
			return index;
		}

		descriptor<S...> sub(size_t ind) const {
			if(ind < this->extends[0]){
				std::array<size_t, dim - 1> str;
				Util::ArrayUtils::arr_copy(this->strides, str, 1, dim - 1);
				return descriptor<S...>(this->offset + ind * this->strides[0], std::move(str));
			}
			else {
				PRINT_ERR("subscript out of bounds", PRIORITY_HALT, CHANNEL_GENERAL_DEBUG)
			}
		}
	};

	template<size_t A>
	/// <summary>
	/// Class for accessing elements from multidimensional arrays saved in a on dimensional array.
	/// </summary>
	class descriptor<A> {
		static constexpr size_t dim = 1;
		size_t offset = 0;
	public:

		std::array<size_t, dim> extends;
		std::array<size_t, dim> strides;

		descriptor(size_t offset, std::array<size_t, dim>&& strides) {
			this->offset = offset;
			this->extends = { A };
			this->strides = std::forward<std::array<size_t, dim>>(strides);
		}

		template<typename S,
			typename = std::enable_if<std::is_convertible<S, size_t>::value>::type>
			size_t operator()(S t) const {
			return this->offset + this->strides[0] * t;
		}

	};

	template<typename T, size_t N>
	struct comp_init {
		using type = std::initializer_list<typename comp_init<T, N - 1>::type >;
	};

	template<typename T>
	struct comp_init<T, 1> {
		using type = std::initializer_list<T>;
	};

	template<typename T, size_t N>
	using comp_initializer = typename comp_init<T, N>::type;

	template<typename T, size_t A, size_t... S>
	struct comp;

	template<typename T, size_t A, size_t... S>
	/// <summary>
	/// This class represents a reference to a multi dimensional array with elements of type T.
	/// The dimensional extends are given via template arguments
	/// </summary>
	class comp_ref {
		using value_type = T;
		static constexpr size_t dim = sizeof...(S) + 1;

	private:
		value_type* data;
		descriptor<A, S...> descr;

	public:

		comp_ref(value_type* base, descriptor<A, S...>&& mv) : descr{ mv } {
			this->data = base;
		}

		comp_ref& operator=(const comp_initializer<T, 1>& ref) {
			auto it = ref.begin();
			size_t ind = 0;
			while (it != ref.end() && ind < A) {
				this->operator[](ind) =*it;
				++ind;
				++it;
			}
		}
		
		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp_ref& operator=(const comp<F, A, S...>& ref);

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp_ref& operator=(const comp_ref<F, A, S...>& ref);


		comp_ref<T, S...> operator[](size_t ind) {
			return comp_ref<T, S...>(this->data, this->descr.sub(ind));
		}

		const comp_ref<T, S...> operator[](size_t ind) const {
			return comp_ref<T, S...>(this->data, this->descr.sub(ind));
		}
	};

	template<typename T, size_t A>
	/// <summary>
	/// This class represents a reference to a multi dimensional array with elements of type T.
	/// The dimensional extends are given via template arguments
	/// </summary>
	class comp_ref<T, A> {
		using value_type = T;

	private:
		value_type* data;
		descriptor<A> descr;

	public:

		comp_ref(value_type* base, descriptor<A>&& mv) : descr{ mv } {
			this->data = base;
		}

		comp_ref& operator=(const comp_initializer<T, 1>& ref) {
			auto it = ref.begin();
			size_t ind = 0;
			while (it != ref.end() && ind < A) {
				this->operator[](ind) = *it;
				++ind;
				++it;
			}
			return *this;
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp_ref& operator=(const comp<F, A>& ref);

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp_ref& operator=(const comp_ref<F, A>& ref);


		T& operator[](size_t ind) {
			return this->data[this->descr(ind)];
		}

		const T& operator[](size_t ind) const {
			return this->data[this->descr(ind)];
		}

	};

	template<typename T, size_t A, size_t... S>
	/// <summary>
	/// This class represents a multi dimensional array with elements of type T.
	/// The dimensional extends are given via template arguments
	/// </summary>
	class comp {
		using value_type = T;
		static constexpr size_t dim = sizeof...(S) + 1;

	private:
		std::array<T, descriptor_size<A, S...>::value> data = { T() };
		descriptor<A, S...> descr;

	public:

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp(const comp<F, A, S...>& ref) {
			for (uint32 t = 0; t < A; ++t) {
				this->operator[](t) = ref[t];
			}
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp(const comp_ref<F, A, S...>& ref) {
			for (uint32 t = 0; t < A; ++t) {
				this->operator[](t) = ref[t];
			}
		}

		comp() : descr{ descriptor<A, S...>(0, std::array<size_t, dim>()) } {
			size_t extends[dim] = { A, S... };
			size_t carry = 1;
			for (size_t t = 0; t < dim; t++) {
				this->descr.strides[dim - 1 - t] = carry;
				carry *= extends[dim - 1 - t];
			}
		}

		comp(comp_initializer<T, dim>&& ref) : comp() {
			auto it = ref.begin();
			size_t ind = 0;
			while (it != ref.end() && ind < A) {
				this->operator[](ind) = std::move(*it);
				++ind;
				++it;
			}
		}

		comp& operator=(comp_initializer<T, dim>&& ref) {
			auto it = ref.begin();
			size_t ind = 0;
			while (it != ref.end() && ind < A) {
				this->operator[](ind) = std::move(*it);
				++ind;
				++it;
			}
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp& operator=(const comp_ref<F, A, S...>&);

		comp_ref<T, S...> operator[](size_t ind) {
			return comp_ref<T, S...>(data.data(), this->descr.sub(ind));
		}

		const comp_ref<T, S...> operator[](size_t ind) const {
			return comp_ref<T, S...>(const_cast<T*>(data.data()), this->descr.sub(ind));
		}
	};

	template<typename T, size_t A>
	/// <summary>
	/// This class represents a multi dimensional array with elements of type T.
	/// The dimensional extends are given via template arguments
	/// </summary>
	class comp<T,A> {
		using value_type = T;
		static constexpr size_t dim = 1;

	private:
		std::array<T, A> data = { T()};

	public:

		comp() { }

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp(const comp<F, A>& ref) {
			for (uint32 t = 0; t < A; ++t) {
				this->data[t] = ref[t];
			}
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp(const comp_ref<F, A>& ref) {
			for (uint32 t = 0; t < A; ++t) {
				this->data[t] = ref[t];
			}
		}

		comp(comp_initializer<T,1>&& ref) {
			auto it = ref.begin();
			size_t ind = 0;
			while (it != ref.end() && ind < A) {
				this->data[ind] = std::move( * it);
				++ind;
				++it;
			}
		}

		comp& operator=(comp_initializer<T, 1>&& ref) {
			auto it = ref.begin();
			size_t ind = 0;
			while (it != ref.end() && ind < A) {
				this->data[ind] = std::move( * it);
				++ind;
				++it;
			}
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		comp& operator=(const comp_ref<F, A>&);

		T& operator[](size_t ind) {
			return data[ind];
		}

		const T& operator[](size_t ind) const {
			return data[ind];
		}

	};
	
	template<
		typename T, 
		size_t A, 
		size_t... S >
	template<typename F,
		typename>
	comp_ref<T, A, S...>& comp_ref<T, A, S...>::operator=(const comp<F, A, S...>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->operator[](i) = ref[i];
		}
		return *this;
	}

	template<
		typename T,
		size_t A,
		size_t... S >
	template<typename F,
		typename>
	comp_ref<T, A, S...>& comp_ref<T, A, S...>::operator=(const comp_ref<F, A, S...>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->operator[](i) = ref[i];
		}

		return *this;
	}

	template<
		typename T,
		size_t A>
	template<typename F,
		typename>
	comp_ref<T, A>& comp_ref<T, A>::operator=(const comp<F, A>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->data[this->descr(i)] = ref[i];
		}

		return *this;
	}

	template<
		typename T,
		size_t A >
	template<typename F,
		typename>
	comp_ref<T, A>& comp_ref<T, A>::operator=(const comp_ref<F, A>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->data[this->descr(i)] = ref[i];
		}
		return *this;
	}

	template<
		typename T,
		size_t A >
	template<typename F,
		typename>
	comp<T, A>& comp<T, A>::operator=(const comp_ref<F, A>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->data[i] = ref[i];
		}
		return *this;
	}

	template<
		typename T,
		size_t A,
		size_t... S>
	template<typename F,
		typename>
	comp<T, A, S...>& comp<T, A, S...>::operator=(const comp_ref<F, A, S...>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->operator[](i) = ref[i];
		}
		return *this;
	}

	//###########################################################################################################################
	template<typename T, size_t A, size_t B >
	class matrix;

	template<typename T, size_t A>
	/// <summary>
	/// This class represent a vector the size is given by the template argument
	/// </summary>
	class vector : public comp<T, A> {
	public:
		using comp<T, A>::comp;

		operator matrix<T, A, 1>();

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		vector operator*(F scale) const {
			vector<T, A> cpy = *this;
			for (size_t t = 0; t < A; t++) {
				cpy[t] *= scale;
			}
			return cpy;
		}

		template<typename F,
			typename G,
			typename = std::enable_if<std::is_convertible<F, T>::value& std::is_convertible<T, G>::value>::type>
			vector operator*(const vector<F, A>& scale) const {
			G val;
			for (size_t t = 0; t < A; ++t) {
				val += this->operator[](t) * scale[t];
			}
			return val;
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
			vector operator+(const vector<F, A>& scale) const {
			vector cpy = *this;
			for (size_t t = 0; t < A; ++t) {
				cpy[t] += scale[t];
			}
			return cpy;
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		vector operator-(const vector<F, A>& scale) const {
			vector cpy = *this;
			for (size_t t = 0; t < A; ++t) {
				cpy[t] -= scale[t];
			}
			return cpy;
		}


	};

	template<typename T, size_t A, size_t B >
	/// <summary>
	/// This class represent a matrix the size is given by the template argument
	/// </summary>
	class matrix : public comp<T, A, B> {
	public:
		using comp<T, A, B>::comp;

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type>
		matrix<T, A, B> operator+(const matrix<F, A, B>& ref) {
			matrix<T, A, B> cpy = *this;
			for (size_t a = 0; a < A; ++a) {
				for (size_t b = 0; b < B; ++b) {
					cpy[a][b] += ref[a][b];
				}
			}
			return cpy;
		}

		template<typename F,
			size_t C,
		typename = std::enable_if<std::is_convertible<F, T>::value>::type >
			matrix<T, A, C> operator*(const matrix<F, B, C>& ref) {
			matrix<T, A, C> val;
			for (size_t a = 0; a < A; ++a) {
				for (size_t c = 0; c < C; ++c) {
					for (size_t b = 0; b < B; ++b) {
						val[a][c] += (*this)[a][b] * ref[b][c];
					}
				}
			}
			return val;
		}

		template<typename F,
			typename = std::enable_if<std::is_convertible<F, T>::value>::type >
		vector<T, A> operator*(const vector<F, B>& ref) {
			vector<T, A> val;
			for (size_t a = 0; a < A; ++a) {
				for (size_t b = 0; b < B; ++b) {
						val[a] += (*this)[a][b] * ref[b];
				}
			}
			return val;
		}

		matrix<T, B, A> transpose() {
			matrix<T, A, B> trans;
			for (size_t a = 0; a < A; ++a) {
				for (size_t b = 0; b < B; ++b) {
					trans[b][a] = (*this)[a][b];
				}
			}
		}

	};

	template<typename T,
		size_t A >
	vector<T,A>::operator matrix<T, A, 1>() {
		matrix<T, A, 1> ref;
		for (size_t i = 0; i < A; ++i) {
			ref[i][0] = (*this)[i];
		}
		return ref;
	}

}