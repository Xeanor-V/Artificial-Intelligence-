#include "HMM.hpp"

namespace ducks
{
    HMM::HMM(int numState,int numEmi,VI obs):
    tranMat(numState), emiMat(numState), iniState()
    {
        srand(time(NULL));
        double coef;
        for(int i = 0 ; i < numState; i++)
        {
            coef = 0;
            for(int j = 0 ; j < numState; j++)
            {
                double x = rand();
                tranMat[i].push_back(x);
                coef+=x;
            }
            coef = 1/coef;
            for(int j = 0 ; j < numState; j++) tranMat[i][j] *= coef;
        }

        for(int i = 0 ; i < numState; i++)
        {
            coef = 0;
            for(int j = 0 ; j < numEmi; j++)
            {
                double x = rand();
                emiMat[i].push_back(x);
                coef+=x;
            }
            coef = 1/coef;
            for(int j = 0 ; j < numEmi; j++) emiMat[i][j] *= coef;
        }


        coef = 0;
        for(int i = 0 ; i < numState; i++)
        {
            double x = rand();
            iniState.push_back(x);
            coef+=x;
        }
        coef = 1/coef;
        for(int i = 0 ; i < numState; i++) iniState[i] *= coef;

        this -> obs = obs;
    }    
    HMM::HMM(VVLD tranMat, VVLD emiMat, VLD iniState, VI obs)
    {
        this -> tranMat = tranMat;
        this -> emiMat = emiMat;
        this -> iniState = iniState;
        this -> obs = obs;
    }
    long double HMM::Scalar_Product(VLD A, VLD B)
    {
        long double res = 0 ;
        for(int i = 0 ; i < A.size(); i++)
        res += ( A[i] * B[i] );
        return res;
    }
    VVLD HMM::Matrix_Sum(VVLD A, VVLD B)
    {
        for(int i = 0 ; i < A.size(); i++)
            for(int j = 0 ; j < A[i].size(); j++) A[i][j] += B[i][j];
        return A;
    }
    VVLD HMM::Matrix_Division(VVLD A, double B)
    {
        for(int i = 0 ; i < A.size(); i++)
            for(int j = 0 ; j < A[i].size(); j++) A[i][j] /= B;
        return A;
    }
    VLD HMM::Vector_Sum(VLD A, VLD B)
    {
        for(int i = 0; i < A.size(); i++) A[i] += B[i];
        return A;
    }
    VLD HMM::Vector_Division(VLD A, double B)
    {
        for(int i = 0; i < A.size(); i++) A[i] /= B;
        return A;
    }
    VLD HMM::Next_Emmision()
    {
        VLD res1,res2;  
        for(int i = 0 ; i < tranMat.size(); i++ )
        {
            res1.push_back(this->HMM::Scalar_Product(tranMat[i],iniState));
        }
        for(int i = 0 ; i < emiMat.size(); i++ )
        {
            res2.push_back(this->HMM::Scalar_Product(emiMat[i],res1));
        }
        return res2;
    }
    VLD HMM::Element_Wise_Product(VLD A, VLD B)
    {
        VLD res;
        for (int i = 0; i < A.size(); i++)
        {
            res.push_back(A[i]*B[i]);
        }
        return res;
    }
    long double HMM::Prob_Emmision_Sequence(VI obs)
    {
        // Initialization
        VLD alpha = iniState;
		/*
		this -> Print_HMM();
        cerr<<"Observations: \n";
        for(int i = 0 ; i < obs.size(); i++)
        {
            cerr<<obs[i]<<' ';
        }
        cerr<<'\n';
		*/
		
		for (int i = 0; i < iniState.size(); i++)
		{
			alpha[i] *= emiMat[i][obs[0]];
		}
        for (int i = 1; i < obs.size(); i++)
        {
            //alpha = this->HMM::Next_Alpha(tranMat, alpha, emiMat[obs[i]]);
            VLD res1(tranMat.size());
			VLD res2(tranMat.size());  
            for(int j = 0 ; j < tranMat.size(); j++ )
            {
				res1[j] = 0;
				for (int k = 0; k < tranMat.size(); k++)
					res1[j] += alpha[k] * tranMat[k][j];
            }
			for (int j = 0; j < iniState.size(); j++)
				res2[j] = emiMat[j][obs[i]] * res1[j];
            alpha = res2;
        }
        long double res = 0;
        for (int i = 0; i < alpha.size(); i++)
        {
            res += alpha[i];
        }
        return res;
    }
    pair<long double, int> HMM::Best_Index_Vector(VLD vec)
    {   
        long double aux = -1e9;
        int index = -1;
        for(int j = 0 ; j < vec.size(); j++) 
        {
            if(aux < vec[j])
            {
                aux = vec[j];
                index = j;
            }
        }
        return make_pair(aux,index);
    }
    DeltaTable HMM::Estimate_Sequence_Of_States()
    {
        // VLD delta = this->HMM::Element_Wise_Product(iniState, emiMat[obs[0]]);
		VLD delta = iniState;
		for (int i = 0; i < iniState.size(); i++)
		{
			delta[i] *= emiMat[i][obs[0]];
		}
        vector< int > bestPossible;
        DeltaTable DeltaResults; 
        // Delta procedure (Forward algorithm)
        for (int i = 1; i < obs.size(); i++)
        {
			vector< pair<long double,int> > res;
            for(int j = 0; j < tranMat.size(); j++)
            {
                VLD aux;
                //long double maxi = -1e9;
                for(int k = 0; k < tranMat[j].size(); k++)
                	aux.push_back(tranMat[k][j] * delta[k] * obs[i]) ; // or obs[j] ? non sense
                res.push_back(this->HMM::Best_Index_Vector (aux) );
            }
            for(int j = 0 ; j < res.size(); j++)
				delta[j] = res[j].first;
            DeltaResults.push_back(res);
        }
        return DeltaResults;
    }
    VI HMM::Backtracking(DeltaTable DeltaResults)
    {
        // Print_DeltaTable(DeltaResults);
		//cerr<<"size deltaresults = "<<DeltaResults.size()<<", "<<DeltaResults[0].size()<<"\n";
		//cerr<<"size tranMat = "<<tranMat.size()<<", "<<tranMat[0].size()<<"\n";

		/*for(int i = 0 ; i < DeltaResults.size(); i++)
        {
            for(int j = 0 ; j < DeltaResults[i].size();j++)
            	cerr<<"("<<DeltaResults[i][j].first<<", "<<DeltaResults[i][j].second<<") ";
            cerr<<"\n";
        }
		cerr<<"\n";*/
		// Backtracking
        double maxi = -1e9;
        int index = -1;
        // Finding the max of the last result
        for(int i = 0; i < DeltaResults[0].size(); i++)
        {   
			long double aux =  DeltaResults[DeltaResults.size() -1 ][i].first;
            if( aux > maxi)
            {
                maxi = aux;
                index = i;
            }
        }
        VI backtracking;
        backtracking.push_back(index);
        index = DeltaResults[DeltaResults.size() -1 ][index].second;
        backtracking.push_back(index);
        // Going back in the results
        for(int i = DeltaResults.size() - 2; i>=0; i--)
        {
			backtracking.push_back(DeltaResults[i][index].second);
            index = DeltaResults[i][index].second;
        }
        return backtracking;
    }
    void HMM::Estimate_Model(int maxIters)
    {
        int i,j,t;
        int T = obs.size();
        int N = iniState.size();
        int M = emiMat[0].size();
        VLD c(T);
        VVLD alpha(T, VLD(N)); // dim = (T,N)
        VVLD beta(T, VLD(N));  // dim = (T,N)
        VVLD gamma2D(T, VLD(N)); // dim = (T,N)
        vector< VVLD > gamma3D(T, VVLD(N, VLD(N))); // dim = (T,N,N)
        
        // cerr<<"Variables ok\n";
        // 1. Initialization ======================
        int iters = 0;
        long double oldLogProb = -DBL_MAX;
        
        // cerr<<"Initialization ok\n";
        // cerr<<"alpha[T-1][N-1] = "<<alpha[T-1][N-1]<<"\n";
        //int z = 200;
        while (true)
        {
            // ========================================
            // 2. The alpha-pass ======================
            // ========================================
            // compute alpha_0(i)
            c[0] = 0;
            for (i=0; i<N ; i++)
            {
                alpha[0][i] = iniState[i] * emiMat[i][obs[0]];
                c[0] += alpha[0][i];
            }
    
            // scale the alpha_0(i)
            c[0] = 1.0/c[0];
            for (i=0; i<N ; i++)
            {
                alpha[0][i] = c[0] * alpha[0][i];
            }
            
            // cerr<<"foobar ok\n";
            // compute alpha_t(i)
            for (t=1; t<T ; t++)
            {
                c[t] = 0.0;
                for (i=0; i<N ; i++)
                {
                    alpha[t][i] = 0.0;
                    for (j=0; j<N; j++)
                    {
                        alpha[t][i] += alpha[t-1][j]*tranMat[j][i];
                    }
                    alpha[t][i] = alpha[t][i] * emiMat[i][obs[t]];
                    c[t] = c[t] + alpha[t][i];
                }
                // scale alpha_t(i)
                c[t] = 1.0/c[t];
                for (i=0; i<N ; i++)
                {
                    alpha[t][i] = c[t]*alpha[t][i];
                }
            }
            // cerr<<"Alpha pass ok\n";
            // ========================================
            // 3. The beta-pass =======================
            // ========================================
        
            // Let beta_T−1(i) = 1, scaled by c_T−1
            for (i=0; i<N ; i++)
            {
                beta[T-1][i] = c[T-1];
            }
        
            // beta-pass
            for (t=T-2 ; t>=0 ; t--)
            {
                for (i=0; i<N ; i++)
                {
                    beta[t][i] = 0.0;
                    for (j=0; j<N; j++)
                    {
                        beta[t][i] += tranMat[i][j] * emiMat[j][obs[t+1]] * beta[t+1][j];
                    }
                    // scale beta_t(i) with same scale factor as alpha_t(i)
                    beta[t][i] = c[t] * beta[t][i];
                }
            }
            // cerr<<"Beta pass ok\n";
            // ===============================================================
            // 4. Compute gamma_t(i, j) and gamma_t(i) =======================
            // ===============================================================
            long double denom;
            for (t=0; t<T-1; t++)
            {
                denom = 0.0;
                for (i=0; i<N ; i++)
                {
                    for (j=0; j<N; j++)
                    {
                        denom += alpha[t][i] * tranMat[i][j] * emiMat[j][obs[t+1]] * beta[t+1][j];
                    }
                }
                for (i=0; i<N ; i++)
                {
                    gamma2D[t][i] = 0.0;
                    for (j=0; j<N; j++)
                    {
                        gamma3D[t][i][j] = (alpha[t][i] * tranMat[i][j] * emiMat[j][obs[t+1]] * beta[t+1][j]) / denom;
                        gamma2D[t][i] += gamma3D[t][i][j];
                    }
                }
            }
            // Special case for gamma_T−1(i)
            denom = 0.0;
            for (i=0; i<N ; i++)
            {
                denom += alpha[T-1][i];
            }
            for (i=0; i<N ; i++)
            {
                gamma2D[T-1][i] = alpha[T-1][i] / denom;
            }
            
            // cerr<<"Gamma computations ok\n";
            // ==================================================
            // 5. Re-estimate tranMat, emiMat and iniState =======================
            // ==================================================
            // re-estimate iniState
            for (i=0; i<N ; i++)
            {
                iniState[i] = gamma2D[0][i];
            }
        
            long double numer;
            // re-estimate tranMat
            for (i=0; i<N ; i++)
            {
                for (j=0; j<N; j++)
                {
                    numer = 0.0;
                    denom = 0.0;
                    for (t=0; t<T-1; t++)
                    {
                        numer += gamma3D[t][i][j];
                        denom += gamma2D[t][i];
                    }
                    tranMat[i][j] = numer/denom;
                }
            }
        
            // re-estimate emiMat
            for (i=0; i<N ; i++)
            {
                for (j=0; j<M; j++)
                {
                    numer = 0.0;
                    denom = 0.0;
                    for (t=0; t<T; t++)
                    {
                        if (obs[t] == j)
                        {
                            numer += gamma2D[t][i];
                        }
                        denom += gamma2D[t][i];
                    }
                    emiMat[i][j] = numer/denom;
                }
            }
            
            // ======================================================
            // 6. Compute log[P (O | lambda)] =======================
            // ======================================================
            long double logProb = 0.0;
            for (i=0; i<T ; i++)
            {
                logProb += log(c[i]);
            }
            logProb = -logProb;
            
            // ===============================================================
            // 7. To iterate or not to iterate, that is the question... ======
            // ===============================================================
            iters++;
            if (iters<maxIters && logProb > oldLogProb)
                oldLogProb = logProb;
            else
                break;
        }
        this -> HMM::iniState = iniState;
        this -> HMM::tranMat = tranMat;
        this -> HMM::emiMat = emiMat;
        //return make_pair(iniState, make_pair(tranMat,emiMat));
    }
    void HMM::Print_HMM()
    {
        cerr<<"Transition matrix: \n";
        for(int i = 0 ; i < tranMat.size(); i++)
        {
            for(int j = 0 ; j < tranMat[i].size();j++)
            cerr<<tranMat[i][j]<<' ';
            cerr<<'\n';
        }
        cerr<<"Emision Matrix: \n";
        for(int i = 0 ; i < emiMat.size(); i++)
        {
            for(int j = 0 ; j < emiMat[i].size();j++)
            cerr<<emiMat[i][j]<<' ';
            cerr<<'\n';
        }
        cerr<<"Initial state probability: \n";
        for(int i = 0 ; i < iniState.size(); i++)
        {
            cerr<<iniState[i]<<' ';
        }
        cerr<<'\n';
		/*
        cerr<<"Observations: \n";
        for(int i = 0 ; i < obs.size(); i++)
        {
            cerr<<obs[i]<<' ';
        }
        cerr<<'\n';
		*/
    }
	void Print_DeltaTable(DeltaTable tab)
	{
        for(int i = 0 ; i < tab.size(); i++)
        {
            for(int j = 0 ; j < tab[i].size();j++)
            	cerr<<"("<<tab[i][j].first<<", "<<tab[i][j].second<<") ";
            cerr<<"\n";
        }
		cerr<<"\n";
	}
    HMM HMM::Avg_HMM(vector<HMM> hmms, HMM previous, double weight)
    {
        VVLD avgTran = hmms[0].tranMat;
        for(int i = 1; i < hmms.size(); i++)
        {
            avgTran = this -> HMM::Matrix_Sum(avgTran,hmms[i].tranMat);
        }
        avgTran = this -> HMM::Matrix_Sum( this -> HMM::Matrix_Division(previous.tranMat,1/weight),avgTran);
        avgTran = this -> HMM::Matrix_Division(avgTran,weight + hmms.size());
        VVLD avgEmi = hmms[0].emiMat;
        for(int i = 1; i < hmms.size(); i++)
        {
            avgEmi = this -> HMM::Matrix_Sum(avgEmi, hmms[i].emiMat);
        }
        avgEmi = this -> HMM::Matrix_Sum( this-> HMM::Matrix_Division(previous.emiMat,1/weight),avgEmi);
        avgEmi = this -> HMM::Matrix_Division(avgEmi,weight + hmms.size());
        VLD avgState = hmms[0].iniState;
        for(int i = 1; i < hmms.size(); i++)
        {
            avgState = this -> HMM::Vector_Sum(avgState, hmms[i].iniState);
        }
        avgState = this -> HMM::Vector_Sum( this-> HMM::Vector_Division(previous.iniState, 1/weight), avgState);
        avgState = this -> HMM::Vector_Division(avgState,weight + hmms.size());
        vector<int> auxObs;
        HMM aux(avgTran,avgEmi,avgState,auxObs);
        return aux;
    }

}


