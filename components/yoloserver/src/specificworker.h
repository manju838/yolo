/*
 *    Copyright (C) 2017 by YOUR NAME HERE
 *
 *    This file is part of RoboComp
 *
 *    RoboComp is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    RoboComp is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RoboComp.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
       \brief
       @author authorname
*/

#ifndef SPECIFICWORKER_H
#define SPECIFICWORKER_H

#include <genericworker.h>
#include <innermodel/innermodel.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <mutex>
#include <queue> 
#include <tuple>
#include <thread>
#include <chrono>
#include "/home/pbustos/software/darknet/include/darknet.h"	

// static network *ynet;
// static clock_t ytime1;
// static float ynms;
// static int yframe = 3;
// static float **predictions;
// static float *yavg;
// static int ytotal = 0; 		

extern "C" 
{
	int size_network(network *net);
	void remember_network(network *net);
	void avg_predictions(network *net);
	char** get_labels(char *);
}

class FPSCounter
{
	public:
		FPSCounter()
		{
			begin = std::chrono::high_resolution_clock::now();
		}
		void print( int &cont, const unsigned int msPeriod = 1000)  
		{
			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration<double>(end - begin).count() * 1000;
			if( elapsed > msPeriod)
			{
				std::cout << "Epoch time = " << elapsed/cont << "ms. Fps = " << cont << std::endl;
				begin = std::chrono::high_resolution_clock::now();
				cont = 0;
			}
		}
		std::chrono::time_point<std::chrono::high_resolution_clock> begin;
};

class SpecificWorker : public GenericWorker
{
		Q_OBJECT
		public:
			SpecificWorker(MapPrx& mprx);
			~SpecificWorker();
			bool setParams(RoboCompCommonBehavior::ParameterList params);
			int processImage(const  TImage &img);
			
		public slots:
			void compute();

		private:
			yolo::image createImage(const TImage& src);
			void init_detector(); 
			detection* detectLabels(const TImage &img, int requestid, float thresh, float hier_thresh);
			
			template<typename T>
			struct ImgSafeBuffer
			{
				unsigned int id=0;
				std::mutex mut;
				std::queue<std::tuple<int, T>> myqueue;
				unsigned int push(const T &img)
				{
						std::lock_guard<std::mutex> lock(mut);
						myqueue.push(std::make_tuple(id, img));
						id++;
						return id-1;
				};
				std::tuple<int, T> popIfNotEmpty() 
				{
					std::lock_guard<std::mutex> lock(mut);
					if(myqueue.empty())
						return std::make_tuple(-1, T());
					auto res = std::tuple(myqueue.front());	//move constructor
					myqueue.pop();
					return res;															//move constructor
				};
				std::size_t size()
				{
					std::lock_guard<std::mutex> lock(mut);
					return myqueue.size();
				}
			};

			ImgSafeBuffer<TImage> lImgs;
			InnerModel *innerModel;
			char** names;
			clock_t ytime1;
			network *ynet;
	};

#endif
	
