#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>

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
const unsigned int N = 5;
const double learning_rate = 0.1;
const unsigned int test_time = 2000;

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
    srand(time(NULL));
    get_corpus();
    compress_corpus_into_voca();
    init_neural_network( N );
    train( test_time );

    unsigned int test_case = corpus.size();
    for(unsigned int test_case = 0; test_case < corpus.size(); test_case ++)
    {
        init_input_layer(test_case);
        feed_forwarding(test_case);

        cout << "\tinput: ";
        for(auto it = corpus[test_case].input_words.begin();
            it != corpus[test_case].input_words.end();
            it++)
        {
            cout << '\t' << (*it);
        }
        cout << endl;
        cout << "\toutput: ";
        for(unsigned int i = 0; i < voca.size(); i ++)
        {
            if(output_layer.value(i) > 0.1f)
                cout << '\t' << voca_index2word[i] << '(' << output_layer.value(i) << ')';
        }
        cout << endl;
    }
    return 0;
}

void init_input_layer(unsigned int corpus_index)
{
    input_layer.value = VectorXd::Constant(voca.size(), 0.f);

    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it++)
    {
        input_layer.value(voca_word2index[(*it)]) =
            1.f
            / (double)corpus[corpus_index].input_words.size();
    }
}
void feed_forwarding(unsigned int corpus_index)
{
    // input2hidden
    hidden_layer.value = VectorXd::Constant(N, 0.f);
    
    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it++)
    {
        for(unsigned int i = 0; i < N; i ++)
        {
            hidden_layer.value(i) +=
                weight_input2hidden(voca_word2index[(*it)], i)
                / (double)corpus[corpus_index].input_words.size();
        }
    }

    // hidden2output
    double exp_sum = 0.f;
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        output_layer.value(i) = 0.f;
        for(unsigned int j = 0; j < N; j ++)
        {
            output_layer.value(i) +=
                hidden_layer.value(j)
                * weight_hidden2output(j, i);
        }
        output_layer.value(i) = exp(output_layer.value(i));
        if(isinf(output_layer.value(i)))
        {
            output_layer.value(i) = (double)numeric_limits<double>::max();
        }
        exp_sum += output_layer.value(i);
    }
    if(isinf(exp_sum))
    {
        exp_sum = (double)numeric_limits<double>::max();
    }

    // apply sigmoid on output
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        output_layer.value(i) /= exp_sum;
    }
}
void backpropagation(unsigned int corpus_index)
{
    // get loss value
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        output_layer.error(i) = output_layer.value(i);
    }
    for(auto it = corpus[corpus_index].output_words.begin();
        it != corpus[corpus_index].output_words.end();
        it++)
    {
        output_layer.error(voca_word2index[(*it)]) -=
            1.f
            / (double)corpus[corpus_index].output_words.size();
    }

    // get gradient hidden2output
    hidden_layer.gradient = VectorXd::Constant(N, 0.f);
    
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        for(unsigned int j = 0; j < N; j ++)
        {
            gradient_hidden2output(j, i) =
                output_layer.error(i) * hidden_layer.value(j);
            hidden_layer.gradient(j) +=
                output_layer.error(i)
                * weight_hidden2output(j, i);
        }
    }

    // get gradient input2hidden
    gradient_input2hidden = MatrixXd::Constant(voca.size(), N, 0.f);

    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it ++)
    {
        for(unsigned int i = 0; i < N; i ++)
        {
            gradient_input2hidden(voca_word2index[(*it)], i) =
                hidden_layer.gradient(i)
                / (double)corpus[corpus_index].input_words.size();
        }
    }
}
void apply_weight(void)
{
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        for(unsigned int j = 0; j < N; j ++)
        {
            weight_input2hidden(i, j) -=
                learning_rate
                * gradient_input2hidden(i, j);
            weight_hidden2output(j, i) -=
                learning_rate
                * gradient_hidden2output(j, i);
        }
    }
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
    for(unsigned int i = 0; i < train_count; i ++)
    {
        train_implement(i % corpus.size());
    }
    cout << "train() finished" << endl;
}

void init_neural_network(unsigned int hidden_layer_neuron_size)
{
    cout << "init_neural_network() begin" << endl;

    input_layer.value = VectorXd::Random(voca.size());
    hidden_layer.value = VectorXd::Random(hidden_layer_neuron_size);
    hidden_layer.gradient = VectorXd::Random(hidden_layer_neuron_size);
    output_layer.value = VectorXd::Random(voca.size());
    output_layer.error = VectorXd::Random(voca.size());

    weight_input2hidden = MatrixXd::Random(voca.size(), hidden_layer_neuron_size);
    gradient_input2hidden = MatrixXd::Random(voca.size(), hidden_layer_neuron_size);
    weight_hidden2output = MatrixXd::Random(hidden_layer_neuron_size, voca.size());
    gradient_hidden2output = MatrixXd::Random(hidden_layer_neuron_size, voca.size());

    /*input_layer.value.resize(voca.size(), 1);
    hidden_layer.value.resize(hidden_layer_neuron_size, 1);
    hidden_layer.gradient.resize(hidden_layer_neuron_size, 1);
    output_layer.value.resize(voca.size(), 1);
    output_layer.error.resize(voca.size(), 1);

    weight_input2hidden.resize(voca.size(), hidden_layer_neuron_size);
    gradient_input2hidden.resize(voca.size(), hidden_layer_neuron_size);
    weight_hidden2output.resize(hidden_layer_neuron_size, voca.size());
    gradient_hidden2output.resize(hidden_layer_neuron_size, voca.size());*/

    cout << "init_neural_network() finished" << endl;
}

void compress_corpus_into_voca(void)
{
    cout << "compress_corpus_into_voca() begin" << endl;
    for(auto it = corpus.begin(); it != corpus.end(); it++)
    {
        cout << "\tinput:";
        for(auto input = (*it).input_words.begin(); input != (*it).input_words.end(); input++)
        {
            cout << '\t' << (*input);
            voca.insert((*input));
        }
        cout << endl;
        cout << "\toutput:";
        for(auto output = (*it).output_words.begin(); output != (*it).output_words.end(); output++)
        {
            cout << '\t' << (*output);
            voca.insert((*output));
        }
        cout << endl;
        cout << endl;
    }
    cout << "\tvoca: ";
    unsigned int index = 0;
    for(auto it = voca.begin(); it != voca.end(); it++, index++)
    {
        cout << '\t' << (*it);
        voca_word2index[(*it)] = index;
        voca_index2word[index] = (*it);
    }
    cout << endl;
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