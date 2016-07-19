/**	Ideia do algoritmo
	....
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <time.h>
#include <list>
#include <limits.h>
using namespace std;

void timeused(double *time)
{
	static double tstart, tend, tprev;

	if (time == NULL) {
		clock(); /* one extra call to initialize clock */
		tstart = tprev = clock();
	}
	else {
		tend = clock();
		if (tend < tprev) tstart -= ULONG_MAX; /* wraparound occured */
		tprev = tend;
		*time = (tend - tstart) / CLOCKS_PER_SEC; /* convert to seconds */
	}
}

class Solucao {
public:
	list<float> tamanho_viga;
	float capacidade_forma;
	int tempo;

	Solucao operator=(Solucao _S){
		capacidade_forma = _S.capacidade_forma;
		tempo = _S.tempo;
		for (list<float>::iterator it = _S.tamanho_viga.begin(); it != _S.tamanho_viga.end(); it++){
			list<float>::iterator _it = tamanho_viga.begin();
			_it = it;
			_it++;
		}
		return *this;
	}
};

class Empacotamento {
protected:
	int m, n, t;           /** #"pacotes" #itens*/
	vector<float> c, l;        /** capacidades, tamanhos*/
	vector<int> d;
	vector<Solucao> sol_aux, solucao_global;
	float z, z_global;
public:

	Empacotamento(const char* filename) {
		z_global = 100000;
		solucao_global.resize(m);
		ifstream input(filename, ifstream::in);
		if (input.fail()) {
			cerr << "     File \"" << filename << "\" not found." << endl;
			exit(1);
		}
		//ler instancia
		input >> m >> n >> t;
		c.resize(m);
		l.resize(n);
		d.resize(n);
		for (int i = 0; i < m; ++i)
			input >> c[i];
		for (int i = 0; i < n; ++i)
			input >> l[i];
		for (int i = 0; i < n; ++i)
			input >> d[i];
		input.close();
	}

	void avaliacao(){
		z = 0;
		int g_m = m*t;
		int _d = 0;
		for (int i = 0; i < t; ++i) {
			float sum = 0;
			for (int j = 0; j < g_m; ++j) {
				if (sol_aux[j].tempo == i) {
					float _uso = 0;
					for (list<float>::iterator it = sol_aux[j].tamanho_viga.begin(); it != sol_aux[j].tamanho_viga.end(); it++) {
						_uso += *it;
						_d++;
					}
					sum +=  sol_aux[j].capacidade_forma - _uso;
				}
			}
			if(i == t-1)
				z = sum;
		}

		if(z >= z_global){
			z_global = z;

			for(int i = 0; i < m; ++i)
				solucao_global[i] = sol_aux[i];
		}

	}

	void imprimir_solu_arq() {
		ofstream arq;
		int g_m = m*t;
		arq.open("solucao.txt");
		z = 0;
		int _d = 0;
		arq << endl << "Solucao:	" << endl;
		for (int i = 0; i < t; ++i) {
		    float sum = 0;
			arq << "	Intervalo de tempo " << i << endl;
			for (int j = 0; j < g_m; ++j) {
				if (sol_aux[j].tempo == i) {
					float _uso = 0;
					arq << "		Forma de capacidade : " << sol_aux[j].capacidade_forma << endl << "			vigas = ";
					for (list<float>::iterator it = sol_aux[j].tamanho_viga.begin(); it != sol_aux[j].tamanho_viga.end(); it++) {
						arq << *it << " ";
						_uso += *it;
						_d++;
					}
					sum +=  sol_aux[j].capacidade_forma - _uso;
					arq << " Perda na forma = " << sol_aux[j].capacidade_forma - _uso  << endl;
				}
			}
			if(i == t-1)
				z = sum;
			arq << "  Perda no tempo de cura = " << sum << endl;
			arq << endl;
		}

		arq << endl << "F.O = z = " << z << endl;

		if (_d == n)
			arq << "Demanda atingida!" << endl;
		else
			arq << "Demanda nao atingida! :(" << endl;
		arq.close();
		cout <<endl << "Z = " << z << endl<<  "Solucao criada no arquivo solucao.txt" << endl;
	}

	void Resolver() {

		//Eh criado um vetor auxiliar para "c" e para "l";
		//Para ficar num formado que tenho "m" * "t" formas
		//	e #total_demandas e vigas. Para ficar em empacotamento.
		vector<float >l_aux, c_aux, solucao;
		int g_m = m*t;

		c_aux.resize(m*t);
		int sum_d = 0;
		for (int i = 0; i < n; ++i)
			sum_d += d[i];

		l_aux.resize(sum_d);

		//c_aux eh preenchido: cada forma aparece t vezes nele
		int x = 0;
		for (int i = 0; i < t; ++i) {
			for (int j = 0; j < m; ++j) {
				c_aux[x] = c[j];
				++x;
			}
		}

		//l_aux eh preenchido: cada viga aparece "numero de sua demanda" vezes.
		x = 0;
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < d[i]; j++) {
				l_aux[x] = l[i];
				++x;
			}
		}

		//Ordeno l_aux para atuar de forma gulosa
		stable_sort(l_aux.begin(), l_aux.end());

		//solucao é o vetor que guarda a capacidade usada da viga em c_aux
		//seto o tamanho do vetor e solucoes
		solucao.resize(g_m);

		//inicializo todas as solucoes com 0
		for (int x = 0; x < m*t; x++)
			solucao[x] = 0;

		//guardo os valores originais de m e n
		//g_m = m;
		//g_n = n;
		//atualizo m e n para os tamanho de c_aux e l_aux agora para facilitar
		//m = m*t;
		//n = sum_d;

		//contador de em qual periodo de tempo eu estou
		int contador_t = 0;

		//bool para dizer se adicionou alguma viga a forma ou n
		bool adicionou;

		//sol_aux eh redimensionada, ela server para facilitar na hora a impressao da solucao
		sol_aux.resize(g_m);

		//vetor auxiliar para calcular a perda no momento
		//eh inicializado com a capacidade total das formas em cada tempo
		vector<float> perda;
		perda.resize(t);
		float sum_c = 0;
		for (int i = 0; i < m; ++i)
			sum_c += c[i];
		for (int i = 0; i < t; ++i)
			perda[i] = sum_c;

		//inicio os periodos de tempo de todas as solucoes em -1
		for (int i = 0; i < m*t; ++i)
			sol_aux[i].tempo = -1;

		//variavel auxiliar para saber se ja foram percorridas todas as formas
		int deu_loop = 0;

		//inicializacao do iterador
		int i = 0;

		//guarda viga de menor e maior comprimento
		float menor, maior;

		//enquanto ainda tiver viga para ser feita
		while (!l_aux.empty()) {
			//inicialização (ainda n mexeu na forma)
			adicionou = false;
				if (l_aux.size() == 1) {
					if (l_aux.front() <= c_aux[i]) {
						sol_aux[i].tamanho_viga.push_back(l_aux.front());
						solucao[i] += l_aux.front();
						adicionou = true;
						break;
					}
					else {
						i++;
						if (i % m - 1 == 0 && i != 0)
							contador_t++;
					}

				}else{
					menor = l_aux[0];
					maior = l_aux[l_aux.size() - 1];

					if (menor + maior + solucao[i] <= c_aux[i]) {
						solucao[i] += menor + maior;
						sol_aux[i].tamanho_viga.push_back(menor);
						sol_aux[i].tamanho_viga.push_back(maior);
						adicionou = true;
						l_aux.erase(l_aux.begin());
						l_aux.pop_back();
					}
					else {
						if (maior + solucao[i] <= c_aux[i]) {
							solucao[i] += maior;
							sol_aux[i].tamanho_viga.push_back(maior);
							adicionou = true;
							l_aux.pop_back();
						}
						else {
							if (menor + solucao[i] <= c_aux[i]) {
								sol_aux[i].tamanho_viga.push_back(menor);
								solucao[i] += menor;
								adicionou = true;
								l_aux.erase(l_aux.begin());
							}
							else {
								i++;
								if (i % m == 0 && i != 0)
									contador_t++;
							}

						}
					}
				}
			//se foi adicionada vigas a forma, atualizo a solucao
			if(adicionou) {
				sol_aux[i].capacidade_forma = c_aux[i];
				sol_aux[i].tempo = contador_t;
			}
		}//while
	}
	~Empacotamento() {
	}
};


int main() {
	double time;

	Empacotamento Prob("problema.txt");

	timeused(NULL);

	Prob.Resolver();

	timeused(&time);

	cout << endl <<"tempo gasto: " << time << endl << endl;

	Prob.imprimir_solu_arq();
	system("pause");
	return 0;
}
