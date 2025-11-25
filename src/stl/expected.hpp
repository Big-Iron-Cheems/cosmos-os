#pragma once

#include <algorithm>
#include <type_traits>
#include <utility>

namespace cosmos::stl {
    /// @brief Represents an unexpected error value for Expected.
    template <class E>
    class Unexpected {
        static_assert(!std::is_reference_v<E>, "E must not be a reference type.");
        static_assert(!std::is_void_v<E>, "E must not be void.");

      private:
        E error_;

      public:
        constexpr Unexpected(const Unexpected&) = default;
        constexpr Unexpected(Unexpected&&) = default;

        template <class Err = E>
            requires(!std::is_same_v<std::remove_cvref_t<Err>, Unexpected> && !std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t> &&
                     std::is_constructible_v<E, Err>)
        constexpr explicit Unexpected(Err&& e) noexcept(std::is_nothrow_constructible_v<E, Err>) : error_(static_cast<Err&&>(e)) {}

        template <class... Args>
            requires std::is_constructible_v<E, Args...>
        constexpr explicit Unexpected(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
            : error_(static_cast<Args&&>(args)...) {}

        constexpr Unexpected& operator=(const Unexpected&) = default;
        constexpr Unexpected& operator=(Unexpected&&) = default;

        constexpr const E& error() const& noexcept {
            return error_;
        }
        constexpr E& error() & noexcept {
            return error_;
        }
        constexpr const E&& error() const&& noexcept {
            return static_cast<const E&&>(error_);
        }
        constexpr E&& error() && noexcept {
            return static_cast<E&&>(error_);
        }

        constexpr void swap(Unexpected& other) noexcept(std::is_nothrow_swappable_v<E>) {
            using std::swap;
            swap(error_, other.error_);
        }

        friend constexpr bool operator==(const Unexpected& lhs, const Unexpected& rhs) noexcept(noexcept(lhs.error_ == rhs.error_)) {
            return lhs.error_ == rhs.error_;
        }
    };

    template <class E>
    Unexpected(E) -> Unexpected<E>;

    template <class E>
    constexpr void swap(Unexpected<E>& lhs, Unexpected<E>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }


    template <class T, class E>
    class Expected {
        static_assert(!std::is_same_v<T, void>, "Use expected<void, E> specialization.");
        static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
        static_assert(!std::is_reference_v<E>, "E must not be a reference type.");

      private:
        union storage_t {
            T value;
            E error;

            constexpr storage_t() noexcept {}
            constexpr ~storage_t() noexcept {}
        } storage_;

        bool has_value_ = false;

        constexpr void destroy() noexcept {
            if (has_value_) {
                storage_.value.~T();
            } else {
                storage_.error.~E();
            }
        }

      public:
        constexpr Expected(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T>) : has_value_(true) {
            new (&storage_.value) T(v);
        }

        constexpr Expected(T&& v) noexcept(std::is_nothrow_move_constructible_v<T>) : has_value_(true) {
            new (&storage_.value) T(static_cast<T&&>(v));
        }

        constexpr Expected(const Unexpected<E>& u) noexcept(std::is_nothrow_copy_constructible_v<E>) : has_value_(false) {
            new (&storage_.error) E(u.error());
        }

        constexpr Expected(Unexpected<E>&& u) noexcept(std::is_nothrow_move_constructible_v<E>) : has_value_(false) {
            new (&storage_.error) E(static_cast<E&&>(u.error()));
        }

        constexpr Expected(const Expected& other) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                           std::is_nothrow_copy_constructible_v<E>)
            : has_value_(other.has_value_) {
            if (has_value_)
                new (&storage_.value) T(other.storage_.value);
            else
                new (&storage_.error) E(other.storage_.error);
        }

        constexpr Expected(Expected&& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
            : has_value_(other.has_value_) {
            if (has_value_)
                new (&storage_.value) T(static_cast<T&&>(other.storage_.value));
            else
                new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
        }

        ~Expected() noexcept {
            destroy();
        }

        // Assignment operators

        constexpr Expected& operator=(const Expected& other) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                                      std::is_nothrow_copy_constructible_v<E> &&
                                                                      std::is_nothrow_copy_assignable_v<T> &&
                                                                      std::is_nothrow_copy_assignable_v<E>) {
            if (this == &other) return *this;

            if (has_value_ && other.has_value_) {
                storage_.value = other.storage_.value;
            } else if (!has_value_ && !other.has_value_) {
                storage_.error = other.storage_.error;
            } else {
                destroy();
                has_value_ = other.has_value_;
                if (has_value_)
                    new (&storage_.value) T(other.storage_.value);
                else
                    new (&storage_.error) E(other.storage_.error);
            }
            return *this;
        }

        constexpr Expected& operator=(Expected&& other) noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                                 std::is_nothrow_move_constructible_v<E> &&
                                                                 std::is_nothrow_move_assignable_v<T> &&
                                                                 std::is_nothrow_move_assignable_v<E>) {
            if (this == &other) return *this;

            if (has_value_ && other.has_value_) {
                storage_.value = static_cast<T&&>(other.storage_.value);
            } else if (!has_value_ && !other.has_value_) {
                storage_.error = static_cast<E&&>(other.storage_.error);
            } else {
                destroy();
                has_value_ = other.has_value_;
                if (has_value_)
                    new (&storage_.value) T(static_cast<T&&>(other.storage_.value));
                else
                    new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
            }
            return *this;
        }

        constexpr Expected& operator=(const T& v) noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                           std::is_nothrow_copy_assignable_v<T>) {
            if (has_value_) {
                storage_.value = v;
            } else {
                destroy();
                has_value_ = true;
                new (&storage_.value) T(v);
            }
            return *this;
        }

        constexpr Expected& operator=(T&& v) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
            if (has_value_) {
                storage_.value = static_cast<T&&>(v);
            } else {
                destroy();
                has_value_ = true;
                new (&storage_.value) T(static_cast<T&&>(v));
            }
            return *this;
        }

        constexpr Expected& operator=(const Unexpected<E>& u) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                       std::is_nothrow_copy_assignable_v<E>) {
            if (!has_value_) {
                storage_.error = u.error();
            } else {
                destroy();
                has_value_ = false;
                new (&storage_.error) E(u.error());
            }
            return *this;
        }

        constexpr Expected& operator=(Unexpected<E>&& u) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                  std::is_nothrow_move_assignable_v<E>) {
            if (!has_value_) {
                storage_.error = static_cast<E&&>(u.error());
            } else {
                destroy();
                has_value_ = false;
                new (&storage_.error) E(static_cast<E&&>(u.error()));
            }
            return *this;
        }

        // Accessors

        /// @brief Checks whether the Expected contains a value.
        [[nodiscard]] constexpr bool has_value() const noexcept {
            return has_value_;
        }

        /// @brief Checks whether the Expected contains a value.
        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return has_value_;
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr T& value() & noexcept {
            return storage_.value;
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const T& value() const& noexcept {
            return storage_.value;
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr T&& value() && noexcept {
            return static_cast<T&&>(storage_.value);
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const T&& value() const&& noexcept {
            return static_cast<const T&&>(storage_.value);
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr E& error() & noexcept {
            return storage_.error;
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const E& error() const& noexcept {
            return storage_.error;
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr E&& error() && noexcept {
            return static_cast<E&&>(storage_.error);
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const E&& error() const&& noexcept {
            return static_cast<const E&&>(storage_.error);
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr T& operator*() & noexcept {
            return storage_.value;
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const T& operator*() const& noexcept {
            return storage_.value;
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr T&& operator*() && noexcept {
            return static_cast<T&&>(storage_.value);
        }

        /// @brief Returns the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const T&& operator*() const&& noexcept {
            return static_cast<const T&&>(storage_.value);
        }

        /// @brief Returns a pointer to the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr T* operator->() noexcept {
            return &storage_.value;
        }

        /// @brief Returns a pointer to the contained value.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const T* operator->() const noexcept {
            return &storage_.value;
        }

        /// @brief Returns the contained value if present, otherwise returns default_value.
        template <class U>
        [[nodiscard]] constexpr T value_or(U&& default_value) const& noexcept(std::is_nothrow_copy_constructible_v<T> &&
                                                                              std::is_nothrow_constructible_v<T, U>) {
            return has_value_ ? storage_.value : static_cast<T>(static_cast<U&&>(default_value));
        }

        /// @brief Returns the contained value if present, otherwise returns default_value.
        template <class U>
        [[nodiscard]] constexpr T value_or(U&& default_value) && noexcept(std::is_nothrow_move_constructible_v<T> &&
                                                                          std::is_nothrow_constructible_v<T, U>) {
            return has_value_ ? static_cast<T&&>(storage_.value) : static_cast<T>(static_cast<U&&>(default_value));
        }

        /// @brief Returns the contained error if present, otherwise returns default_error.
        template <class G = E>
        [[nodiscard]] constexpr E error_or(G&& default_error) const& noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                              std::is_nothrow_constructible_v<E, G>) {
            return !has_value_ ? storage_.error : static_cast<E>(static_cast<G&&>(default_error));
        }

        /// @brief Returns the contained error if present, otherwise returns default_error.
        template <class G = E>
        [[nodiscard]] constexpr E error_or(G&& default_error) && noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                          std::is_nothrow_constructible_v<E, G>) {
            return !has_value_ ? static_cast<E&&>(storage_.error) : static_cast<E>(static_cast<G&&>(default_error));
        }

        /// @brief Destroys any contained value or error and constructs a new value in-place.
        template <class... Args>
        constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            destroy();
            has_value_ = true;
            new (&storage_.value) T(static_cast<Args&&>(args)...);
            return storage_.value;
        }

        /// @brief Swaps the contents with another Expected.
        constexpr void swap(Expected& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E> &&
                                                      std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<E>) {
            if (has_value_ && other.has_value_) {
                using std::swap;
                swap(storage_.value, other.storage_.value);
            } else if (!has_value_ && !other.has_value_) {
                using std::swap;
                swap(storage_.error, other.storage_.error);
            } else {
                Expected* with_value = has_value_ ? this : &other;
                Expected* with_error = has_value_ ? &other : this;

                T tmp_value(static_cast<T&&>(with_value->storage_.value));
                with_value->destroy();
                with_value->has_value_ = false;
                new (&with_value->storage_.error) E(static_cast<E&&>(with_error->storage_.error));

                with_error->destroy();
                with_error->has_value_ = true;
                new (&with_error->storage_.value) T(static_cast<T&&>(tmp_value));
            }
        }

        // Comparison

        friend constexpr bool operator==(const Expected& lhs,
                                         const Expected& rhs) noexcept(noexcept(lhs.storage_.value == rhs.storage_.value) &&
                                                                       noexcept(lhs.storage_.error == rhs.storage_.error)) {
            if (lhs.has_value_ != rhs.has_value_) return false;
            if (lhs.has_value_) return lhs.storage_.value == rhs.storage_.value;
            return lhs.storage_.error == rhs.storage_.error;
        }

        friend constexpr bool operator!=(const Expected& lhs, const Expected& rhs) noexcept(noexcept(lhs == rhs)) {
            return !(lhs == rhs);
        }

        template <class U>
        friend constexpr bool operator==(const Expected& lhs, const U& rhs) noexcept(noexcept(lhs.storage_.value == rhs)) {
            return lhs.has_value_ && lhs.storage_.value == rhs;
        }

        template <class G>
        friend constexpr bool operator==(const Expected& lhs,
                                         const Unexpected<G>& rhs) noexcept(noexcept(lhs.storage_.error == rhs.error())) {
            return !lhs.has_value_ && lhs.storage_.error == rhs.error();
        }
    };

    /// @brief Specialization of Expected for void value type.
    template <class E>
    class Expected<void, E> {
        static_assert(!std::is_reference_v<E>, "E must not be a reference type.");

      private:
        union storage_t {
            E error;
            constexpr storage_t() noexcept {}
            constexpr ~storage_t() noexcept {}
        } storage_;

        bool has_value_ = true;

        constexpr void destroy() noexcept {
            if (!has_value_) {
                storage_.error.~E();
            }
        }

      public:
        // success
        constexpr Expected() noexcept = default;

        // error
        constexpr Expected(const Unexpected<E>& u) noexcept(std::is_nothrow_copy_constructible_v<E>) : has_value_(false) {
            new (&storage_.error) E(u.error());
        }
        constexpr Expected(Unexpected<E>&& u) noexcept(std::is_nothrow_move_constructible_v<E>) : has_value_(false) {
            new (&storage_.error) E(static_cast<E&&>(u.error()));
        }

        constexpr Expected(const Expected& other) noexcept(std::is_nothrow_copy_constructible_v<E>) : has_value_(other.has_value_) {
            if (!has_value_) new (&storage_.error) E(other.storage_.error);
        }

        constexpr Expected(Expected&& other) noexcept(std::is_nothrow_move_constructible_v<E>) : has_value_(other.has_value_) {
            if (!has_value_) new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
        }

        ~Expected() noexcept {
            destroy();
        }

        // Assignment operators

        constexpr Expected& operator=(const Expected& other) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                      std::is_nothrow_copy_assignable_v<E>) {
            if (this == &other) return *this;

            if (!has_value_ && !other.has_value_) {
                storage_.error = other.storage_.error;
            } else if (has_value_ && !other.has_value_) {
                has_value_ = false;
                new (&storage_.error) E(other.storage_.error);
            } else if (!has_value_ && other.has_value_) {
                destroy();
                has_value_ = true;
            }
            return *this;
        }

        constexpr Expected& operator=(Expected&& other) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                 std::is_nothrow_move_assignable_v<E>) {
            if (this == &other) return *this;

            if (!has_value_ && !other.has_value_) {
                storage_.error = static_cast<E&&>(other.storage_.error);
            } else if (has_value_ && !other.has_value_) {
                has_value_ = false;
                new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
            } else if (!has_value_ && other.has_value_) {
                destroy();
                has_value_ = true;
            }
            return *this;
        }

        constexpr Expected& operator=(const Unexpected<E>& u) noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                       std::is_nothrow_copy_assignable_v<E>) {
            if (!has_value_) {
                storage_.error = u.error();
            } else {
                has_value_ = false;
                new (&storage_.error) E(u.error());
            }
            return *this;
        }

        constexpr Expected& operator=(Unexpected<E>&& u) noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                  std::is_nothrow_move_assignable_v<E>) {
            if (!has_value_) {
                storage_.error = static_cast<E&&>(u.error());
            } else {
                has_value_ = false;
                new (&storage_.error) E(static_cast<E&&>(u.error()));
            }
            return *this;
        }

        // Accessors

        /// @brief Checks whether the Expected contains a value (success state).
        [[nodiscard]] constexpr bool has_value() const noexcept {
            return has_value_;
        }

        /// @brief Checks whether the Expected contains a value (success state).
        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return has_value_;
        }

        /// @brief No-op for void specialization.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        constexpr void value() const noexcept {
            // no-op
        }

        /// @brief No-op for void specialization.
        /// @pre has_value() == true. Behavior is undefined if this precondition is violated.
        constexpr void operator*() const noexcept {
            // no-op
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr E& error() & noexcept {
            return storage_.error;
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const E& error() const& noexcept {
            return storage_.error;
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr E&& error() && noexcept {
            return static_cast<E&&>(storage_.error);
        }

        /// @brief Returns the contained error.
        /// @pre has_value() == false. Behavior is undefined if this precondition is violated.
        [[nodiscard]] constexpr const E&& error() const&& noexcept {
            return static_cast<const E&&>(storage_.error);
        }

        /// @brief Returns the contained error if present, otherwise returns default_error.
        template <class G = E>
        [[nodiscard]] constexpr E error_or(G&& default_error) const& noexcept(std::is_nothrow_copy_constructible_v<E> &&
                                                                              std::is_nothrow_constructible_v<E, G>) {
            return !has_value_ ? storage_.error : static_cast<E>(static_cast<G&&>(default_error));
        }

        /// @brief Returns the contained error if present, otherwise returns default_error.
        template <class G = E>
        [[nodiscard]] constexpr E error_or(G&& default_error) && noexcept(std::is_nothrow_move_constructible_v<E> &&
                                                                          std::is_nothrow_constructible_v<E, G>) {
            return !has_value_ ? static_cast<E&&>(storage_.error) : static_cast<E>(static_cast<G&&>(default_error));
        }

        /// @brief Destroys any contained error and transitions to success state.
        constexpr void emplace() noexcept {
            if (!has_value_) {
                destroy();
                has_value_ = true;
            }
        }

        /// @brief Swaps the contents with another Expected.
        constexpr void swap(Expected& other) noexcept(std::is_nothrow_move_constructible_v<E> && std::is_nothrow_swappable_v<E>) {
            if (!has_value_ && !other.has_value_) {
                using std::swap;
                swap(storage_.error, other.storage_.error);
            } else if (has_value_ && !other.has_value_) {
                has_value_ = false;
                new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
                other.destroy();
                other.has_value_ = true;
            } else if (!has_value_ && other.has_value_) {
                other.has_value_ = false;
                new (&other.storage_.error) E(static_cast<E&&>(storage_.error));
                destroy();
                has_value_ = true;
            }
        }

        // Comparison

        friend constexpr bool operator==(const Expected& lhs,
                                         const Expected& rhs) noexcept(noexcept(lhs.storage_.error == rhs.storage_.error)) {
            if (lhs.has_value_ != rhs.has_value_) return false;
            if (lhs.has_value_) return true;
            return lhs.storage_.error == rhs.storage_.error;
        }

        friend constexpr bool operator!=(const Expected& lhs, const Expected& rhs) noexcept(noexcept(lhs == rhs)) {
            return !(lhs == rhs);
        }

        template <class G>
        friend constexpr bool operator==(const Expected& lhs,
                                         const Unexpected<G>& rhs) noexcept(noexcept(lhs.storage_.error == rhs.error())) {
            return !lhs.has_value_ && lhs.storage_.error == rhs.error();
        }
    };

    /// @brief Swaps two Expected objects.
    template <class T, class E>
    constexpr void swap(Expected<T, E>& lhs, Expected<T, E>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

} // namespace cosmos::stl
