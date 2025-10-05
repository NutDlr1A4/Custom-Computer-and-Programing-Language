/*
* The 'Lookahead' container is basically just a container that is at a spesific index, and
* you can either read the value (or check if it exists thanks to std::optional) at that index, 
* or 'eat' it. Eating a value returns it and increments the index.
* 
* I made this class because I realized I would need a similar behaviour in different modules of the assembler.
* For example:
*	- Reading characters one after one from a string
*	- Reading tokens one after one from a vector of tokens
*	and maybe more later...
*/

export module lookahead;

import std;

export
template<typename T, typename Container = std::vector<T>> // Not proud of this template, but it works for now
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
	std::size_t Index() const {
		return index;
	}
	void Reset() {
		Seek(0);
	}
private:
	const Container& src;
	std::size_t index = 0;
};