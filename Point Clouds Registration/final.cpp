#include <iostream>
#include <vector>
#include <math.h>
#include <numeric>
#include <armadillo>
#include <fstream>
using namespace std;
using namespace arma;

#define MAX_DIM 3


void split(const string&s, const char* delim, vector<string> &v){
	char *dup = strdup(s.c_str());
	char *token = strtok(dup, delim);
	while(token != NULL){
		v.push_back(string(token));
		token = strtok(NULL, delim);
	}
	free(dup);
}

//Build a KD-Tree for nearest distance
struct kd_node_t{
    double x[MAX_DIM];
    struct kd_node_t *left, *right;
};

inline double
dist(struct kd_node_t *a, struct kd_node_t *b, int dim)
{
    double t, d = 0;
    while (dim--) {
        t = a->x[dim] - b->x[dim];
        d += t * t;
    }
    return d;
}
inline void swap(struct kd_node_t *x, struct kd_node_t *y) {
    double tmp[MAX_DIM];
    memcpy(tmp,  x->x, sizeof(tmp));
    memcpy(x->x, y->x, sizeof(tmp));
    memcpy(y->x, tmp,  sizeof(tmp));
}
 
    struct kd_node_t*
find_median(struct kd_node_t *start, struct kd_node_t *end, int idx)
{
    if (end <= start) return NULL;
    if (end == start + 1)
        return start;
 
    struct kd_node_t *p, *store, *md = start + (end - start) / 2;
    double pivot;
    while (1) {
        pivot = md->x[idx];
 
        swap(md, end - 1);
        for (store = p = start; p < end; p++) {
            if (p->x[idx] < pivot) {
                if (p != store)
                    swap(p, store);
                store++;
            }
        }
        swap(store, end - 1);
 
        /* median has duplicate values */
        if (store->x[idx] == md->x[idx])
            return md;
 
        if (store > md) end = store;
        else        start = store;
    }
}
 
    struct kd_node_t*
make_tree(struct kd_node_t *t, int len, int i, int dim)
{
    struct kd_node_t *n;
 
    if (!len) return 0;
 
    if ((n = find_median(t, t + len, i))) {
        i = (i + 1) % dim;
        n->left  = make_tree(t, n - t, i, dim);
        n->right = make_tree(n + 1, t + len - (n + 1), i, dim);
    }
    return n;
}
 
int visited;
 
void nearest(struct kd_node_t *root, struct kd_node_t *nd, int i, int dim,
        struct kd_node_t **best, double *best_dist)
{
    double d, dx, dx2;
 
    if (!root) return;
    d = dist(root, nd, dim);
    dx = root->x[i] - nd->x[i];
    dx2 = dx * dx;
 
    visited ++;
 
    if (!*best || d < *best_dist) {
        *best_dist = d;
        *best = root;
    }
 
    /* if chance of exact match is high */
    if (!*best_dist) return;
 
    if (++i >= dim) i = 0;
 
    nearest(dx > 0 ? root->left : root->right, nd, i, dim, best, best_dist);
    if (dx2 >= *best_dist) return;
    nearest(dx > 0 ? root->right : root->left, nd, i, dim, best, best_dist);
}
//mat is the matrix format containing in library 'armadillo'
mat calRotationMatrix(vec q){
	mat ret(3, 3, fill::zeros);
	double q0 = q(0);
	double q1 = q(1);
	double q2 = q(2);
	double q3 = q(3);
	ret(0,0) = pow(q0, 2) + pow(q1, 2) - pow(q2, 2) - pow(q3, 2); 
	ret(0,1) = 2*(q1*q2 - q0*q3);
	ret(0,2) = 2*(q1*q3 + q0*q2);
	ret(1,0) = 2*(q1*q2 + q0*q3);
	ret(1,1) = pow(q0, 2) + pow(q2, 2) - pow(q1, 2) - pow(q3, 2);
	ret(1,2) = 2*(q2*q3 - q0*q1);
	ret(2,0) = 2*(q1*q3 - q0*q2);
	ret(2,1) = 2*(q2*q3 + q0*q1);
	ret(2,2) = pow(q0, 2) + pow(q3, 2) - pow(q1, 2) - pow(q2, 0);
	return ret;
}

vector<mat> calRegistration(mat P0, mat Yk){
	//Calculate the accumulate sum
	mat up = cumsum(P0);
	up = up / P0.n_cols;
	mat uy = cumsum(Yk);
	uy = uy / Yk.n_cols;

	//% is the Schur product
	mat Epy = P0.t() % Yk - up.t() % uy;
	Epy = Epy / P0.n_cols;

	double tr = trace(Epy);

	mat A = Epy - Epy.t();
	mat B = Epy + Epy.t() - (eye<mat>(3,3) * tr);
	mat Qepy(4, 4, fill::zeros);
	Qepy(0,0) = tr; Qepy(0,1) = A(1,2); Qepy(0,2) = A(2,0); Qepy(0,3) = A(0,1);
	Qepy(1,0) = A(1,2); Qepy(1,1) = B(0,0); Qepy(1,2) = 0; Qepy(1,3) = 0;
	Qepy(2,0) = A(2,0); Qepy(2,1) = 0; Qepy(2,2) = B(1,1); Qepy(2,3) = 0;
	Qepy(3,0) = A(0,1); Qepy(3,1) = 0; Qepy(3,2) = 0; Qepy(3,3) = B(2,2);

	//Generate the eigenvector and eigenvalue
	vec eigval;
	mat eigvec;
	eig_sym(eigval, eigvec, Qepy);

	mat qr = eigvec.col(0);
	mat qt = (uy.t() - calRotationMatrix(qr) % up.t()).t();
	vector<mat> ret;
	ret.push_back(qr);
	ret.push_back(qt);
	return ret;
}

mat applyRegistration(vector<mat> qk, mat P0){
	mat qr = qk[0];
	mat qt = qk[1];
	return P0 % calRotationMatrix(qr).t() + qt;
}

int calMeanSquareError(mat Yk, mat Pk){
	int ret = accu(pow(Yk - Pk, 2));
	ret = ret / Pk.n_cols;
	return ret;
}

typedef std::vector<double> stdvec;

vector<mat> ICP(mat P0, mat X){
	int k = 10;
	float threadhold = 0.05;
	mat Pk = P0;
	int dk = 10;
	vector<mat> qk; //Quaternion
	stdvec Xv = conv_to<stdvec>::from(X);
	kd_node_t kdTree[] = Xv;
	kd_node_t *root;
	root = make_tree(kdTree, sizeof(kdTree)/sizeof(kdTree[0]), 0, 3);
	for(int i = 0; i < k; ++i){
		cout << "--- iteration "<< i+1 <<"----" <<endl;
		vector<mat> Yk;
		for(int i = 0; i < Pk.n_cols; i++){
			kd_node_t *found;
			double best_dist;
			int visited = 0;
			found = NULL;
			kd_node_t testNode = Pk[i];
			nearest(root, &testNode, 0, 3, &found, &best_dist);
			Yk.push_back(X[found[1]]);
			if (i % int(Pk.n_cols/100) == 0){
				cout <<"calculating closet point set "<< i / int(Pk.n_cols/100);
			}
		}
		cout << endl;
		cout << "Begin calculating registration" <<endl;
		qk = calRegistration(P0, Yk);
		cout << "Done calculating registration" <<endl;
		cout << "Begin applying registration" <<endl;
		Pk = applyRegistration(qk, P0);
		cout << "Done calculating registration" <<endl;
		cout << "Calculating mean sqaure error" << endl;
		int dkNext = calMeanSquareError(Yk, Pk);
		cout << "Error shifting" << abs(dkNext - dk);
		if(abs(dkNext - dk) < threadhold)
			break;
		dk = dkNext;
	}
}

int main(){
	cout << "Loading measured point cloud set" << endl;
	ifstream fin;
	fin.open("pointcloud1.fuse");
	vector<double> P;
	if(fin.is_open()){
		string line;
		while(getline(fin, line)){
			vector<string> s_attr;
			split(line, " ", s_attr);
			for(int i = 0; i < s_attr.size(); ++i){
				P.push_back(stod(s_attr[i]));
			}
		}
	}
	fin.close();
	fin.open("pointcloud2.fuse");
	vector<double> X;
	if(fin.is_open()){
		string line;
		while(getline(fin, line)){
			vector<string> s_attr;
			split(line, " ", s_attr);
			for(int i = 0; i < s_attr.size(); ++i){
				X.push_back(stod(s_attr[i]));
			}
		}
	}
	fin.close();

	cout << "Calculating transformating using ICP" << endl;
	vector<mat> R;
	R = ICP(P, X);
	cout << "Roration Matrix 'R' = " << R[0];
	cout << "Translationg Vector 'T' = " << R[1];
}