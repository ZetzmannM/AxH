#pragma once

#include <array>
#include <tuple>
#include <concepts>

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

		template<typename... S>
			requires all_true<std::is_convertible<S,size_t>...>::value
		size_t operator()(S... t)  const {
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

		template<typename S>
			requires std::is_convertible<S, size_t>::value
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
		requires std::is_default_constructible<T>::value
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
		
		template<typename F>
			requires std::is_convertible<F, T>::value
		comp_ref& operator=(const comp<F, A, S...>& ref);

		template<typename F>
			requires std::is_convertible<F, T>::value
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

		template<typename F>
			requires std::is_convertible<F, T>::value
		comp_ref& operator=(const comp<F, A>& ref) {
			for (size_t i = 0; i < A; i++) {
				this->data[this->descr(i)] = ref[i];
			}

			return *this;
		}

		template<typename F>
			requires std::is_convertible<F, T>::value
		comp_ref& operator=(const comp_ref<F, A>& ref) {
			for (size_t i = 0; i < A; i++) {
				this->data[this->descr(i)] = ref[i];
			}
			return *this;
		}


		T& operator[](size_t ind) {
			return this->data[this->descr(ind)];
		}

		const T& operator[](size_t ind) const {
			return this->data[this->descr(ind)];
		}

	};

	template<typename T, size_t A, size_t... S>
		requires std::is_default_constructible<T>::value
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

		template<typename F>
			requires std::is_convertible<F, T>::value
		comp(const comp<F, A, S...>& ref) {
			for (uint32 t = 0; t < A; ++t) {
				this->operator[](t) = ref[t];
			}
		}

		template<typename F>
			requires std::is_convertible<F, T>::value
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

		template<typename F>
			requires std::is_convertible<F, T>::value
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

		template<typename F>
			requires std::is_convertible<F, T>::value
		comp(const comp<F, A>& ref) {
			for (uint32 t = 0; t < A; ++t) {
				this->data[t] = ref[t];
			}
		}

		template<typename F>
			requires std::is_convertible<F, T>::value
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

		template<typename F>
			requires std::is_convertible<F, T>::value
		comp& operator=(const comp_ref<F, A>&) {
			for (size_t i = 0; i < A; i++) {
				this->data[i] = ref[i];
			}
			return *this;
		}

		T& operator[](size_t ind) {
			return data[ind];
		}

		const T& operator[](size_t ind) const {
			return data[ind];
		}

	};
	

	//###########################################################################################################################

	template<typename T>
	concept math_type = requires (T a, T b) {
		a + b;
		a - b;
		a * b;
		a += b;
		a *= b;
	};


	template<math_type T, size_t A, size_t B >
	class matrix;

	template<math_type T, size_t A>
	/// <summary>
	/// This class represent a vector the size is given by the template argument
	/// </summary>
	class vector : public comp<T, A> {
	public:
		using comp<T, A>::comp;

		operator matrix<T, A, 1>();

		template<typename F>
			requires std::is_convertible<F, T>::value
		vector operator*(F scale) const;

		template<typename F,
			typename G>
			requires std::is_convertible<F, T>::value
				  && std::is_convertible<T, G>::value
		G operator*(const vector<F, A>& scale);

		template<typename F>
			requires std::is_convertible<F, T>::value
		vector operator+(const vector<F, A>& scale) const;

		template<typename F>
			requires std::is_convertible<F, T>::value
		vector operator-(const vector<F, A>& scale) const;

		template<std::floating_point F>
		F length() const;

	};

	template<math_type T, size_t A, size_t B >
	/// <summary>
	/// This class represent a matrix the size is given by the template argument
	/// </summary>
	class matrix : public comp<T, A, B> {
	public:
		using comp<T, A, B>::comp;

		template<typename F>
			requires std::is_convertible<F, T>::value
		matrix<T, A, B> operator+(const matrix<F, A, B>& ref);

		template<typename F,
			size_t C>
			requires std::is_convertible<F, T>::value
		matrix<T, A, C> operator*(const matrix<F, B, C>& ref);

		template<typename F>
			requires std::is_convertible<F, T>::value
		vector<T, A> operator*(const vector<F, B>& ref);

		matrix<T, B, A> transpose();

	};


//#######################################################################################################################
	
	class vector_math {
		template<math_type T>
		/// <summary>
		/// Calculates the cross product of the given vectors
		/// </summary>
		/// <returns>The cross product</returns>
		vector<T, 3> cross_product(const vector<T, 3>&, const vector<T, 3>&);

		template< 
			std::floating_point T,
			size_t A >
		/// <summary>
		/// project the first vector onto the second. That is it returns a multiple of the second vector.
		/// </summary>
		/// <returns>A projection of the first vector onto the second</returns>
		vector<T, A> project(const vector<T, A>&, const vector<T, A>&);

		template<
			std::floating_point T,
			size_t A >
		/// <returns> A normalized version of the given vector </returns>
		vector<T, A> normalize(const vector<T, A>&);
	};

//########################################################################################################################
	

	template<
		typename T,
		size_t A,
		size_t... S >
		template<typename F>
		requires std::is_convertible<F, T>::value
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
		template<typename F>
		requires std::is_convertible<F, T>::value
	comp_ref<T, A, S...>& comp_ref<T, A, S...>::operator=(const comp_ref<F, A, S...>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->operator[](i) = ref[i];
		}

		return *this;
	}

	template<
		typename T,
		size_t A,
		size_t... S>
		template<typename F>
		requires std::is_convertible<F, T>::value
	comp<T, A, S...>& comp<T, A, S...>::operator=(const comp_ref<F, A, S...>& ref) {
		for (size_t i = 0; i < A; i++) {
			this->operator[](i) = ref[i];
		}
		return *this;
	}


	template<
		math_type T,
		size_t A,
		size_t B>
	template<typename F>
		requires std::is_convertible<F, T>::value
	matrix<T, A, B> matrix<T, A, B>::operator+(const matrix<F, A, B>& ref) {
		matrix<T, A, B> cpy = *this;
		for (size_t a = 0; a < A; ++a) {
			for (size_t b = 0; b < B; ++b) {
				cpy[a][b] += ref[a][b];
			}
		}
		return cpy;
	}

	template<
		math_type T,
		size_t A,
		size_t B>
	template<typename F,
		size_t C>
		requires std::is_convertible<F, T>::value
	matrix<T, A, C> matrix<T, A, B>::operator*(const matrix<F, B, C>& ref) {
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

	template<
		math_type T,
		size_t A,
		size_t B>
	template<typename F>
		requires std::is_convertible<F, T>::value
	vector<T, A> matrix<T, A, B>::operator*(const vector<F, B>& ref) {
		vector<T, A> val;
		for (size_t a = 0; a < A; ++a) {
			for (size_t b = 0; b < B; ++b) {
				val[a] += (*this)[a][b] * ref[b];
			}
		}
		return val;
	}

	template<
		math_type T,
		size_t A,
		size_t B>
	matrix<T, B, A> matrix<T, A, B>::transpose() {
		matrix<T, A, B> trans;
		for (size_t a = 0; a < A; ++a) {
			for (size_t b = 0; b < B; ++b) {
				trans[b][a] = (*this)[a][b];
			}
		}
	}


	template<math_type T,
		size_t A >
	inline vector<T, A>::operator matrix<T, A, 1>() {
		matrix<T, A, 1> ref;
		for (size_t i = 0; i < A; ++i) {
			ref[i][0] = (*this)[i];
		}
		return ref;
	}


	template<math_type T, size_t A>
	template<typename F,
		typename G>
		requires std::is_convertible<F, T>::value
			  && std::is_convertible<T, G>::value
	G vector<T, A>::operator*(const vector<F, A>& scale) {
		G val;
		for (size_t t = 0; t < A; ++t) {
			val += this->operator[](t) * scale[t];
		}
		return val;
	}


	template<math_type T,
		size_t A >
	template<typename F>
		requires std::is_convertible<F, T>::value
	vector<T,A> vector<T,A>::operator*(F scale) const {
		vector<T, A> cpy = *this;
		for (size_t t = 0; t < A; t++) {
			cpy[t] *= scale;
		}
		return cpy;
	}

	template<math_type T,
		size_t A >
	template<typename F>
		requires std::is_convertible<F, T>::value
	vector<T,A> vector<T, A>::operator+(const vector<F, A>& scale) const {
		vector cpy = *this;
		for (size_t t = 0; t < A; ++t) {
			cpy[t] += scale[t];
		}
		return cpy;
	}

	template<math_type T,
		size_t A >
	template<typename F>
		requires std::is_convertible<F, T>::value
	vector<T,A> vector<T,A>::operator-(const vector<F, A>& scale) const {
		vector cpy = *this;
		for (size_t t = 0; t < A; ++t) {
			cpy[t] -= scale[t];
		}
		return cpy;
	}

	template<math_type T,
		size_t A >
	template<std::floating_point F>
	F vector<T,A>::length() const {
		F sum = 0;
		for (size_t t = 0; t < A; ++t) {
			sum += (*this)[t] * (*this)[t];
		}

		//ew
		if constexpr (std::is_same<F, double>) {
			return std::sqrt(sum);
		}
		else if constexpr (std::is_same<F, long double>) {
			return std::sqrtl(sum);
		}
		else {
			return std::sqrtf(sum);
		}
	}

	template<math_type T>
	vector<T, 3> vector_math::cross_product(const vector<T, 3>& a, const vector<T, 3>& b) {
		vector<T, 3> c;
		c[0] = (a[1] * b[2] - a[2] * b[1]);
		c[1] = -(a[0] * b[2] - a[2] * b[0]);
		c[2] = (a[0] * b[1] - a[1] * b[0]);
		return c;
	}

	template<
		std::floating_point T,
		size_t A >
	vector<T, A> vector_math::project(const vector<T, A>& a, const vector<T, A>& b) {
		vector<T, A> bN = normalize<T, A>(b);
		return bN * (bN * a);
	}

	template<
		std::floating_point T,
		size_t A >
	vector<T, A> vector_math::normalize(const vector<T, A>& a) {
		T one = 1;
		return a * (one / a.length<T>());
	}

}