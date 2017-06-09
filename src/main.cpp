#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <ratio>
#include <limits>
#include <chrono>

#include "Eigen/Core"
#include "Eigen/Dense"

using namespace Eigen;
using namespace std;

void get_corpus(void);
void compress_corpus_into_voca(void);
void init_neural_network(unsigned int hidden_layer_neuron_size);
void train(unsigned int train_count);
void init_input_layer(unsigned int corpus_index);
void feed_forwarding(unsigned int corpus_index);
void backpropagation(unsigned int corpus_index);
void apply_weight();

class word_pair
{
public:
    vector<string> input_words, output_words;
    word_pair() {}
};
vector<word_pair> corpus;
set<string> voca;
map<string, unsigned int> voca_word2index;
map<unsigned int, string> voca_index2word;
const unsigned int N = 2000;
const double learning_rate = 0.0001;
const unsigned int test_time = 10000;

#define __LOAD
//#define __SAVE
#define FILE_NAME "weights"

class layer
{
public:
    VectorXd value, gradient, error;
    layer() {}
};

layer input_layer, hidden_layer, output_layer;
MatrixXd weight_input2hidden, weight_hidden2output;
MatrixXd gradient_input2hidden, gradient_hidden2output;

int main(void)
{
    auto start_time = chrono::high_resolution_clock::now();
    setNbThreads(0);
    cout << "The number of threads(" << nbThreads() << ")" << endl;
    initParallel();

    srand(time(NULL));

    string word;

#ifndef __LOAD
    get_corpus();
    compress_corpus_into_voca();
    init_neural_network( N );
    train( test_time );

#else
    ifstream file_in(FILE_NAME);
    size_t voca_size = 0U;
    unsigned int word_idx;
    file_in >> voca_size;
    for(unsigned int idx = 0; idx < voca_size; idx ++)
    {
        file_in >> word >> word_idx;
        voca.insert(word);
        voca_word2index[word] = word_idx;
        voca_index2word[word_idx] = word;
    }
    init_neural_network( N );
    for(unsigned int i = 0; i < N; i ++)
    {
        for(unsigned int j = 0; j < voca_size; j ++)
        {
            file_in >> weight_input2hidden(i, j);
        }
    }
    for(unsigned int i = 0; i < voca_size; i ++)
    {
        for(unsigned int j = 0; j < N; j ++)
        {
            file_in >> weight_hidden2output(i, j);
        }
    }
    file_in.close();
#endif

    do {
        cout << "enter word: ";
        cin >> word;
        if(voca_word2index.find(word) != voca_word2index.end())
        {
            // init input layer
            input_layer.value = VectorXd::Zero(voca.size());
            input_layer.value(voca_word2index[word]) = 1.f;

            feed_forwarding(0);

            for(unsigned int i = 0; i < voca.size(); i ++)
            {
                if(output_layer.value(i) > 0.1f)
                    cout << '\t' << voca_index2word[i] << '(' << output_layer.value(i) << ')';
            }
            cout << endl << endl;
        }
    } while(word.compare("x"));

	using namespace std::chrono;

    auto end_time = chrono::high_resolution_clock::now();
    auto elapsed = duration_cast<duration<double>>(end_time - start_time);
    cout << "Running Time: " << elapsed.count() << "seconds" << endl;

#ifdef __SAVE
    ofstream file_out(FILE_NAME);
    file_out << voca.size() << endl;
    for(auto it = voca.begin(); it != voca.end(); it ++)
    {
        file_out << *it << " " << voca_word2index[*it] << endl;
    }
    file_out << weight_input2hidden;
    file_out << endl;
    file_out << weight_hidden2output;
    file_out.close();
#endif

    return 0;
}

void init_input_layer(unsigned int corpus_index)
{
    input_layer.value = VectorXd::Zero(voca.size());

    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it++)
    {
        input_layer.value(voca_word2index[(*it)]) =
            1.f
            / (double)corpus[corpus_index].input_words.size();
    }
}
double exponential_with_upper_bound(double value)
{
    double _return = exp(value);
    return isinf(_return) ? numeric_limits<double>::max() : _return;
}
void feed_forwarding(unsigned int corpus_index)
{
    // input2hidden
    hidden_layer.value = (weight_input2hidden * input_layer.value).transpose();

    // hidden2output
    output_layer.value = (weight_hidden2output * hidden_layer.value).transpose();

    // get sigmoid
    output_layer.value = output_layer.value.unaryExpr(&exponential_with_upper_bound);
    double divider = output_layer.value.sum();
    if(isinf(divider))
    {
        divider = numeric_limits<double>::max();
    }
    output_layer.value /= divider;
}
void backpropagation(unsigned int corpus_index)
{
    // get loss value
    output_layer.error = output_layer.value;

    for(auto it = corpus[corpus_index].output_words.begin();
        it != corpus[corpus_index].output_words.end();
        it++)
    {
        output_layer.error(voca_word2index[(*it)]) -=
            1.f
            / (double)corpus[corpus_index].output_words.size();
    }

    // get gradient hidden2output
    for(unsigned int i = 0; i < N; i += 1)
    {
        gradient_hidden2output.col(i) = hidden_layer.value(i) * output_layer.error;
    }
    hidden_layer.gradient = (weight_hidden2output.transpose() * output_layer.error).transpose();

    // get gradient input2hidden
    gradient_input2hidden = MatrixXd::Zero(N, voca.size());

    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it ++)
    {
        gradient_input2hidden.col(voca_word2index[*it]) =
            hidden_layer.gradient
            / (double)corpus[corpus_index].input_words.size();
    }
}
void apply_weight(void)
{
    weight_input2hidden -= learning_rate * gradient_input2hidden;
    weight_hidden2output -= learning_rate * gradient_hidden2output;
}

void train_implement(unsigned int corpus_index)
{
    init_input_layer(corpus_index);
    feed_forwarding(corpus_index);
    backpropagation(corpus_index);
    apply_weight();
}

void train(unsigned int train_count)
{
    cout << "train() begin" << endl;

	int trainRate = 0;
    for(unsigned int i = 0; i < train_count; i ++)
    {
        for(unsigned int j = 0; j < corpus.size(); j ++)
            train_implement(j);

		int nowRate = (i+1) * 100 / train_count;
		if (trainRate < nowRate) {
			trainRate = nowRate;
			cout << '\r' << "training... [";
			for (unsigned int j = 1; j <= 10; j++)
			{
				if (trainRate / 10 >= j)
					cout << "*";
				else
					cout << " ";
			}
			cout << "] " << trainRate << "%";

			if (trainRate == 100)
				cout << endl;
		}
			
    }
    cout << "train() finished" << endl;
}

void init_neural_network(unsigned int hidden_layer_neuron_size)
{
    cout << "init_neural_network() begin" << endl;

    input_layer.value = VectorXd::Random(voca.size());
    hidden_layer.value = VectorXd::Random(N);
    hidden_layer.gradient = VectorXd::Random(N);
    output_layer.value = VectorXd::Random(voca.size());
    output_layer.error = VectorXd::Random(voca.size());

    weight_input2hidden = MatrixXd::Random(N, voca.size());
    gradient_input2hidden = MatrixXd::Random(N, voca.size());
    weight_hidden2output = MatrixXd::Random(voca.size(), N);
    gradient_hidden2output = MatrixXd::Random(voca.size(), N);

    cout << "init_neural_network() finished" << endl;
}

void compress_corpus_into_voca(void)
{
    cout << "compress_corpus_into_voca() begin" << endl;
    for(auto it = corpus.begin(); it != corpus.end(); it++)
    {
        for(auto input = (*it).input_words.begin(); input != (*it).input_words.end(); input++)
        {
            voca.insert((*input));
        }
        for(auto output = (*it).output_words.begin(); output != (*it).output_words.end(); output++)
        {
            voca.insert((*output));
        }
    }
    unsigned int index = 0;
    for(auto it = voca.begin(); it != voca.end(); it++, index++)
    {
        voca_word2index[(*it)] = index;
        voca_index2word[index] = (*it);
    }
    cout << "\ttotal voca: " << voca.size() << endl;
    cout << "compress_corpus_into_voca() finished" << endl;
}

pair<string, string> trim(string str, char token)
{
    string head, tail;
    unsigned int index = 0U;
    for(auto it = str.begin(); it != str.end(); it++, index++)
    {
        if((*it) == token)
        {
            head = str.substr(0, index);
            tail = str.substr(index+1, str.length()-index);
            return make_pair(head, tail);
        }
    }
    return make_pair(str, string(""));
}

void get_corpus(void)
{
    cout << "get_corpus() begin" << endl;

    ifstream input_stream("input_SkipGram.txt");
    unsigned int count = 0U;
    while(input_stream.eof() == false)
    {
        count += 1U;
        string str, input, output;
        input_stream >> str;

        auto pair = trim(str, '|');
        input = pair.first;
        output = pair.second;

        word_pair word;

        do {
            pair = trim(input, '^');
            word.input_words.push_back(pair.first);
            input = pair.second;
        } while(input.length());
        do {
            pair = trim(output, '^');
            word.output_words.push_back(pair.first);
            output = pair.second;
        } while(output.length());

        corpus.push_back(word);
    }
    input_stream.close();

    cout << "\ttotal corpus: \t" << count << endl;

    cout << "get_corpus() finished" << endl;
}