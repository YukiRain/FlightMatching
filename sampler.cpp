#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <ctime>
#include <queue>

const int NFLIGHTS = 303;
const int NGATES = 69;
const int MAX_MATCH = 41;

using namespace std;

struct Exception {
	Exception(const char* s): info(s) {}
	Exception(): info("Undefined error!") {}
	string info;
};

template <typename T> struct Mat {
	Mat(T* _d, int r, int c) : data(_d), row(r), col(c) {}
	inline T& operator()(int i, int j) const {
		return this->data[i*col + j];
	}

	T* data;
	int row, col;
};

extern "C" {

/* Monte Carlo sampling
** matchings: shape (753, 41)
** sizes: shape (753)
** timeidx: int32 of shape (753, 2) representing (begin time, end time)
** probs: float32 type shape (753, 41)
** assign: int32 type assignment vevtor of shape (753)
*/
void monte_carlo(const Mat<int>& matchings, const int *sizes, const Mat<float>& probs, int* assign) {
	float epsilon; int delta;
	for (int i = 0; i < NFLIGHTS; i++) {
		do {
			delta = rand() % sizes[i];
			epsilon = rand() / (RAND_MAX + 1e-20);
		} while (epsilon > probs(i, delta));
#ifdef	MY_DEBUG
		if(matchings(i, delta) == -1) {
			printf("Invalid matching\n");
			throw Exception("matching error");
		}
#endif
		assign[i] = matchings(i, delta);
	}
}

/**
** ports: shape (69, K) representing the flight assigned to each port
** sizes: shape (69)
** timeidx: shape (753, 2) representing (begin time, end time) of the i-th flight
** K: maximum value of sizes
** times: sorted vector<int*> of timeidx pointers
**/
int _greedy_search(const vector<pair<int*, int>>& times, int *assign) {
	if (times.empty())
		return 0;
	int tmp = times[0].first[1], res = 1;
	for (int i = 1; i < times.size(); i++) {
		if (times[i].first[0] >= tmp + 9) {
#ifdef	MY_DEBUG
			if(times[i].first[0] >= times[i].first[1]) {
				printf("Invalid time\n");
				for(auto& elem : times)
					printf("(%d--%d) ", *(elem.first), *(elem.first + 1));
				printf("\n");
				throw Exception("Invalid time");
			}
#endif
			tmp = times[i].first[1];
			res += 1;
		} else {
			assign[times[i].second] = -1;
		}
	}

	return times.size() - res;
}

/*
** Definition of interfaces
** Function sampling: return assign
** Function timeidx: return ans of shape (2),
** where ans[0] is the number of failure,
** and ans[1] is the total number of used gates.
*/
void sampling(int* matchings, const int *sizes, float* probs, int* assign) {
	Mat<int> match(matchings, NFLIGHTS, MAX_MATCH);
	Mat<float> proba(probs, NFLIGHTS, MAX_MATCH);
	float epsilon; int delta, i, tmp;

	// srand((unsigned int)time(0));
	for (i = 0; i < NFLIGHTS; i++) {
		do {
			delta = rand() % (sizes[i]);
			epsilon = rand() / (RAND_MAX + 1e-20);
		} while (epsilon > proba(i, delta));

#ifdef	MY_DEBUG
		if(match(i, delta) == -1) {
			printf("Invalid matching!\n");
			for(int j=0; j<NFLIGHTS; j++)
				printf("%d ", match(j, delta));
			throw Exception();
		}
#endif

		assign[i] = match(i, delta);
	}
}

int greedy_search(int* ports, int* sizes, int* timeidx, 
                  int K, int *ans, int *assign, int *matchings=NULL,
				  int *matchsize=NULL) {
	typedef pair<int*, int> info_t;

	Mat<int> timers(timeidx, NFLIGHTS, 2);
	Mat<int> porters(ports, NGATES, K);
	int res = 0, usage = 0;
	for (int i = 0; i < NGATES; i++) {
		vector<info_t> data(sizes[i]);

#ifdef	MY_DEBUG
		if(sizes[i] > 200) {
			for(int i=0; i<NGATES; i++)
				printf("%d ", sizes[i]);
			printf("\n");
			throw Exception();
		}
#endif

		if (sizes[i] != 0)
			usage += 1;
		for (int j = 0; j < sizes[i]; j++) {
			data[j] = info_t(&(timers(porters(i, j), 0)), porters(i, j));
		}
		sort(data.begin(), data.end(), [](const info_t& a, const info_t& b) { 
			return a.first[1] < b.first[1];
		});
		res += _greedy_search(data, assign);

	} // for

	ans[0] = res;
	ans[1] = usage;
}


} // extern "C"


//int main() {
//	
//	return 0;
//}
