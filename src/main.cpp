#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include <cstdlib>
#include <ctime>

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
const int max_value = 32768;

class matrix
{
public:
    double **component = nullptr;
    matrix() {}
    matrix(unsigned int row, unsigned int column)
    {
        resize(row, column);
    }
    void resize(unsigned int row, unsigned int column)
    {
        // delete
        if(component != nullptr) {
            for(unsigned int i = 0; i < row; i ++)
            {
                delete component[ i ];
            }
            delete component;
            component = nullptr;
        }

        // create
        component = new double*[row];
        for(unsigned int i = 0; i < row; i ++)
        {
            component[i] = new double[column];
            for(unsigned int j = 0; j < column; j ++)
            {
                component[i][j] = (((double)(rand()%max_value))-0.5f)/((double)N);
            }
        }
    }
};

class layer
{
public:
    matrix value, gradient, error;
    layer() {}
};

layer input_layer, hidden_layer, output_layer;
matrix weight_input2hidden, weight_hidden2output;
matrix gradient_input2hidden, gradient_hidden2output;

int main(void)
{
    srand(time(NULL));
    get_corpus();
    compress_corpus_into_voca();
    init_neural_network( N );
    train( 2000 );

    const unsigned int test_case = 9;
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
            if(output_layer.value.component[i][0] > 0.f)
                cout << '\t' << voca_index2word[i] << '(' << output_layer.value.component[i][0] << ')';
        }
        cout << endl;
    }

/*    cout << "\tinput_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ ) {
        if(input_layer.value.component[i][0] >= 0.f)
            cout << "\tR";
        else
            cout << "\tB";
    }
    cout << endl;
    cout << "\thidden_layer: ";
    for(unsigned int i = 0; i < N; i ++ ) {
        if(hidden_layer.value.component[i][0] >= 0.f)
            cout << "\tR";
        else
            cout << "\tB";
    }
    cout << endl;
    cout << "\toutput_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ ) {
        if(output_layer.value.component[i][0] >= 0.f)
            cout << "\tR";
        else
            cout << "\tB";
    }
    cout << endl;
    cout << endl;


    cout << "\tweight_input2hidden: " << endl;
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < N; j ++)
        {
            if(weight_input2hidden.component[i][j] >= 0.f)
                cout << "\tR";
            else
                cout << "\tB";
        }
        cout << endl;
    }
    cout << endl;
    cout << "\tweight_hidden2output: " << endl;
    for(unsigned int i = 0; i < N; i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < voca.size(); j ++)
        {
            if(weight_hidden2output.component[i][j] >= 0.f)
                cout << "\tR";
            else
                cout << "\tB";
        }
        cout << endl;
    }
    cout << endl;*/
    return 0;
}

void init_input_layer(unsigned int corpus_index)
{
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        input_layer.value.component[i][0] = 0;
    }
    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it++)
    {
        input_layer.value.component[voca_word2index[(*it)]][0] = 1.f ;/* / corpus[corpus_index].input_words.size(); */
    }
}
void feed_forwarding(unsigned int corpus_index)
{
    // input2hidden
    for(unsigned int i = 0; i < N; i ++)
    {
        hidden_layer.value.component[i][0] = 0.f;
    }
    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it++)
    {
        for(unsigned int i = 0; i < N; i ++)
        {
            hidden_layer.value.component[i][0] +=
                weight_input2hidden.component[voca_word2index[(*it)]][i]
                / (double)corpus[corpus_index].input_words.size();
        }
    }

    // hidden2output
    double exp_sum = 0.f;
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        output_layer.value.component[i][0] = 0.f;
        for(unsigned int j = 0; j < N; j ++)
        {
            output_layer.value.component[i][0] +=
                hidden_layer.value.component[j][0]
                * weight_hidden2output.component[j][i];
        }
        output_layer.value.component[i][0] = exp(output_layer.value.component[i][0]);
        if(isinf(output_layer.value.component[i][0]))
        {
            output_layer.value.component[i][0] = (double)max_value;
        }
        exp_sum += output_layer.value.component[i][0];
    }
    if(isinf(exp_sum))
    {
        exp_sum = (double)max_value;
    }

    // apply sigmoid on output
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        output_layer.value.component[i][0] /= exp_sum;
    }
}
void backpropagation(unsigned int corpus_index)
{
    // get loss value
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        output_layer.error.component[i][0] = output_layer.value.component[i][0];
    }
    for(auto it = corpus[corpus_index].output_words.begin();
        it != corpus[corpus_index].output_words.end();
        it++)
    {
        output_layer.error.component[voca_word2index[(*it)]][0] -= 1.f;
    }

    // get gradient hidden2output
    for(unsigned int i = 0; i < N; i ++)
    {
        hidden_layer.gradient.component[i][0] = 0;
    }
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        for(unsigned int j = 0; j < N; j ++)
        {
            gradient_hidden2output.component[j][i] =
                output_layer.error.component[i][0]
                * hidden_layer.value.component[j][0];
            hidden_layer.gradient.component[j][0] +=
                output_layer.error.component[i][0]
                * weight_hidden2output.component[j][i];
        }
    }

    // get gradient input2hidden
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        for(unsigned int j = 0; j < N; j ++)
        {
            gradient_input2hidden.component[i][j] = 0.f;
        }
    }
    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it ++)
    {
        for(unsigned int i = 0; i < N; i ++)
        {
            gradient_input2hidden.component[voca_word2index[(*it)]][i] =
                hidden_layer.gradient.component[i][0]
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
            weight_input2hidden.component[i][j] -=
                learning_rate
                * gradient_input2hidden.component[i][j];
            weight_hidden2output.component[j][i] -=
                learning_rate
                * gradient_hidden2output.component[j][i];
        }
    }
}

void train_implement(unsigned int corpus_index)
{
    init_input_layer(corpus_index);
    feed_forwarding(corpus_index);
    backpropagation(corpus_index);
    apply_weight();
/*
    cout << "\tinput: ";
    for(auto it = corpus[corpus_index].input_words.begin();
        it != corpus[corpus_index].input_words.end();
        it++)
    {
        cout << '\t' << (*it);
    }
    cout << endl;
    cout << "\toutput: ";
    for(unsigned int i = 0; i < voca.size(); i ++) {
        if(output_layer.value.component[i][0] > 0.5f)
            cout << 't' << voca_index2word[i];
    }
    cout << endl;
    cout << endl;


    cout << "\tinput_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ )
        cout << '\t' << input_layer.value.component[i][0];
    cout << endl;
    cout << "\thidden_layer: ";
    for(unsigned int i = 0; i < N; i ++ )
        cout << '\t' << hidden_layer.value.component[i][0];
    cout << endl;
    cout << "\toutput_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ )
        cout << '\t' << output_layer.value.component[i][0];
    cout << endl;
    cout << "\terror_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ )
        cout << '\t' << output_layer.error.component[i][0];
    cout << endl;
    cout << endl;


    cout << "\tweight_input2hidden: " << endl;
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < N; j ++)
        {
            cout << '\t' << weight_input2hidden.component[i][j];
        }
        cout << endl;
    }
    cout << endl;
    cout << "\tweight_hidden2output: " << endl;
    for(unsigned int i = 0; i < N; i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < voca.size(); j ++)
        {
            cout << '\t' << weight_hidden2output.component[i][j];
        }
        cout << endl;
    }
    cout << endl;

    cout << "\tgradient_input2hidden: " << endl;
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < N; j ++)
        {
            cout << '\t' << gradient_input2hidden.component[i][j];
        }
        cout << endl;
    }
    cout << endl;
    cout << "\tgradient_hidden2output: " << endl;
    for(unsigned int i = 0; i < N; i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < voca.size(); j ++)
        {
            cout << '\t' << gradient_hidden2output.component[i][j];
        }
        cout << endl;
    }
    cout << endl;
    cout << endl;*/
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

    input_layer.value.resize(voca.size(), 1);
    hidden_layer.value.resize(hidden_layer_neuron_size, 1);
    hidden_layer.gradient.resize(hidden_layer_neuron_size, 1);
    output_layer.value.resize(voca.size(), 1);
    output_layer.error.resize(voca.size(), 1);

    weight_input2hidden.resize(voca.size(), hidden_layer_neuron_size);
    gradient_input2hidden.resize(voca.size(), hidden_layer_neuron_size);
    weight_hidden2output.resize(hidden_layer_neuron_size, voca.size());
    gradient_hidden2output.resize(hidden_layer_neuron_size, voca.size());

    cout << "\tinput_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ )
        cout << '\t' << input_layer.value.component[i][0];
    cout << endl;
    cout << "\thidden_layer: ";
    for(unsigned int i = 0; i < hidden_layer_neuron_size; i ++ )
        cout << '\t' << hidden_layer.value.component[i][0];
    cout << endl;
    cout << "\toutput_layer: ";
    for(unsigned int i = 0; i < voca.size(); i ++ )
        cout << '\t' << output_layer.value.component[i][0];
    cout << endl;
    cout << endl;


    cout << "\tweight_input2hidden: " << endl;
    for(unsigned int i = 0; i < voca.size(); i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < hidden_layer_neuron_size; j ++)
        {
            cout << '\t' << weight_input2hidden.component[i][j];
        }
        cout << endl;
    }
    cout << endl;
    cout << "\tweight_hidden2output: " << endl;
    for(unsigned int i = 0; i < hidden_layer_neuron_size; i ++)
    {
        cout << '\t';
        for(unsigned int j = 0; j < voca.size(); j ++)
        {
            cout << '\t' << weight_hidden2output.component[i][j];
        }
        cout << endl;
    }
    cout << endl;

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

    ifstream input_stream("input_KingAndQueen.txt");
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