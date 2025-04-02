export module lookahead;

import std;

export
template<typename T, typename Container = std::vector<T>>
class Lookahead {
public:
	Lookahead(const Container& cont)
		:
		src(cont) {
	}

	std::optional<T> At(std::size_t offset = 0) const {
		if (index + offset >= src.size()) {
			return std::nullopt;
		}
		return src.at(index + offset);
	}

	T Eat() {
		T element = src.at(index);
		index++;
		return element;
	}

	void Seek(std::size_t new_index) {
		index = new_index;
	}
	void Reset() {
		Seek(0);
	}
private:
	const Container& src;
	std::size_t index = 0;
};