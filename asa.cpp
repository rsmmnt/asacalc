#include <random>
#include <iostream>
#include <vector>

using namespace std;

#define NPTS 500 // number of random points


struct float4
{
	float x,y,z;
};


void ReadPosition(std::string fname, int NMax, float4* particles_host)
{
int bloat;
int a,b,c,d;
float x,y,z;
FILE *f = fopen(fname.c_str(),"r");
    int temp;

        for(int j = 0; j < NMax ; j++)
        {
            fscanf(f,"%f %f %f %i %i %i %i\n",&x,&y,&z,&a,&b,&c,&d);
            particles_host[j].x = x;
            particles_host[j].y = y;
            particles_host[j].z = z;
        }

    fclose(f);
}


// Marsaglia's method of generating random point on a sphere

float4 randSpherePt(float4 point, float rad, std::default_random_engine &gen)
{	 
    std::uniform_real_distribution<> dis(std::nextafter(-1.0, std::numeric_limits<double>::max()), 1.0);
	float4 res;
	float x1,x2;
	do{
		x1 = dis(gen);
		x2 = dis(gen);
	}while(x1*x1 + x2*x2 >= 1);
	res.x = rad*2*x1*sqrt(1-x1*x1-x2*x2) + point.x;
	res.y = rad*2*x2*sqrt(1-x1*x1-x2*x2) + point.y;
	res.z = rad*(1 - 2*(x1*x1 + x2*x2)) + point.z;
	return res;	
}

inline float sqlen(float4 &a, float4 &b)
{
	return (a.x - b.x)*(a.x -b.x) + (a.y - b.y)*(a.y -b.y) + (a.z - b.z)*(a.z -b.z); 
	
}

vector<vector<int>> buildList(int rad, float4* crd, int N)
{
	vector<vector<int>> nl(N);
	for(int i = 0; i < N; i++)
	{
		for(int j = i+1; j < N; j++)
		{
			float dsq = sqlen(crd[i], crd[j]);
			if(dsq < rad*rad)
			{
				nl[i].push_back(j);
				nl[j].push_back(i);
			}
		
		}
	}
	
	return nl;
}

float calcSurf(int rad, float4* crd,int N, int start, int end, vector<vector<int>>& nList,  std::default_random_engine &gen)
{
	float sumsurf = 0;
	for(int i = start; i < end; i++)
	{
		float sum = 0;
		for(int j = 0; j < NPTS; j++)
		{
			float4 pt = randSpherePt(crd[i],rad,gen);
			float isSelf = 1; // if this random point does not belong to any other sphere
			for(int k = 0; k < nList[i].size(); k++)
			{
				
				if(sqlen(pt, crd[nList[i][k]]) <= rad*rad && nList[i][k] < end && nList[i][k] >= start)
				{
					isSelf = 0;
				}
			}
			sum+=isSelf;
		}
		sumsurf += sum/NPTS;	
	
	}

	return sumsurf;

}


int main(int argc, char* argv[])
{
	std::random_device rd; 
	std::default_random_engine gen(rd());
	if(argc < 4)
	{
		std::cout << "Three arguments expected - filename, numParticles and rCut";
		return 0;
	}
	int numParticles = atoi(argv[2]);
	std::cout << "numParticles " << numParticles << std::endl;
	std::string fname(argv[1]);
	float rCut = atof(argv[3]);
	std::cout << "filename " << fname.c_str() << std::endl;	
	std::vector<float4> crd(numParticles);
	ReadPosition(fname, numParticles, crd.data());
	vector<vector<int>> nl = buildList(rCut*2,crd.data(),numParticles);

	for(int s = 5; s < numParticles; s+=20 )
	{
		int lag = s;
		float srf = 0;
		int cnt = 0;
		for(int zz = 0; zz < numParticles - s; zz+=lag)
		{
			srf += calcSurf(rCut, crd.data(), numParticles, zz, zz+s, nl, gen);
			cnt++;
		}
		
		std::cout << s << " " << srf/cnt << std::endl;
	}
	return 0;
}

