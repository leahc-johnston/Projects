#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <ctime>
class customerStatus {
	private:
		int current_custNum;
		int current_arrivalTime;
		int startTime;
		int waitTime;
		int serviceTime;
		int responseTime;
	public:
		customerStatus(){current_custNum = 0; current_arrivalTime = 0; startTime = 0; waitTime = 0; serviceTime = 0; responseTime = 0;}
		void setCurrent_custNum(int n){current_custNum = n;}
		void setCurrent_arrivalTime(int a){current_arrivalTime = a;}
		void setStartTime(int s){startTime = s;}
		void setWaitTime(int w){waitTime = w;}
		void setServiceTime(int st){serviceTime = st;}
		void setResponseTime(int r){responseTime = r;}

		int getCurrent_custNum(){return current_custNum;}
		int getCurrent_arrivalTime(){return current_arrivalTime;}
		int getStartTime(){return startTime;}
		int getWaitTime(){return waitTime;}
		int getServiceTime(){return serviceTime;}
		int getResponseTime(){return responseTime;}
};
class simulationStats{
	private:
		int totCust;
		int totServiceTime;
		int totWaitTime;
		int maxQueueSize;
		int totResponseTime;
	public:
		simulationStats(){totCust = 0; totServiceTime = 0; totWaitTime = 0; maxQueueSize = 0; totResponseTime = 0;}
		void setTotCust(int tc){totCust = tc;}
		void setTotServiceTime(int tst){totServiceTime = tst;}
		void setTotWaitTime(int twt){totWaitTime = twt;}
		void setMaxQueueSize(int mqs){maxQueueSize = mqs;}
		void setTotResponseTime(int r){totResponseTime = r;}

		float getTotCust(){return totCust;}
		int getTotServiceTime(){return totServiceTime;}
		int getTotWaitTime(){return totWaitTime;}
		int getMaxQueueSize(){return maxQueueSize;}
		int getTotResponseTime(){return totResponseTime;}
};
/*struct customerStatus {
	int current_custNum;
	int current_arrivalTime;
	int startTime;
	int waitTime;
	int serviceTime;
};
struct simulationStats {
	int totCust;
	int totServiceTime;
	int totWaitTime;
	int maxQueueSize;
};*/


using namespace std;

#include "taskNode.h"
#include "project2QueueClass.h"

void createCustomer(Queue *simQueue, int arriveTime, int clock, int duration, int &lastNum);
void checkServerFree(Queue *simQueue, int clock, customerStatus *custStatus, int serveTime, bool *more);
void checkCustComplete(Queue *simQueue, int clock, customerStatus *custStatus, simulationStats *simStats, bool *more);
void printStatus(customerStatus *custStatus);
void printStats(simulationStats *simStats);

int main()
{
	int i, j, k;
	int clock;
	int custNum;
	int duration;
	int a_arriveTime;
	int av_serviceTime;
	bool more;
	int num;
	int lastNum;

	cout << "Enter the duration of the Simulation (in seconds) : \t\n";
	cin >> duration;

	cout << "Enter the average arriving time (in seconds) : \t\n";
	cin >> a_arriveTime;

	cout << "Enter the average servicing time (in seconds) : \t\n";
	cin >> av_serviceTime;

	Queue *simQueue;
	simQueue = createQueue();

	QueueNode *node;

	customerStatus *custStatus;
	custStatus = new customerStatus();

	simulationStats *simStats;
	simStats = new simulationStats();

	srand(time(0));
	clock = 1;
	custNum = 0;
	lastNum = 0;
	more = true;

	while((clock <= duration) || (more=true))
	{

		if(clock < duration)
		{	
			createCustomer(simQueue, a_arriveTime, clock, duration, lastNum);
		}
		checkServerFree(simQueue, clock, custStatus, av_serviceTime, &more);
		checkCustComplete(simQueue, clock, custStatus, simStats, &more);


		if(simQueue->isEmptyQueue() && clock>duration)
		{
			more = false;
			break;
		}
		clock+=1;
	}
	printStats(simStats);
	return 0;
}
void createCustomer(Queue *simQueue, int a_arriveTime, int clock, int duration, int &lastNum){
	QueueNode *node;
	int ar, tasknum;
	ar = 1+ rand() % a_arriveTime;
	if(ar== a_arriveTime)
	{
		cout << "Arrive time equivilant \t" << ar << "\t=\t" << a_arriveTime << endl;
	
		lastNum+=1;
		tasknum = lastNum;
		node = createNode(tasknum, clock);
		node->setCustNum(tasknum);
		node->setArrivalTime(clock);

		simQueue->enqueue(node);

	}
	else
		cout << "No one has arrived\n";	
	return;
}

void checkServerFree(Queue *simQueue, int clock, customerStatus *custStatus, int serveTime, bool *more)
{
	QueueNode *node;
	int t, c, a;
	if(clock > custStatus->getStartTime() + custStatus->getServiceTime()-1) //note may need to be initialized
	{
		if(!simQueue->isEmptyQueue())
		{

			node = simQueue->dequeue();
			c = node->getCustNum();
			custStatus->setCurrent_custNum(c);

			a = node->getArrivalTime();
			custStatus->setCurrent_arrivalTime(a);

			custStatus->setStartTime(clock);

			t = 1+ rand() % serveTime;
			custStatus->setServiceTime(t);

			*more = true;

		}

	}
	else 
		cout << "Either the server isn't free or there is a big problem here! \n";
	return;
}
void checkCustComplete(Queue *simQueue, int clock, customerStatus *custStatus, simulationStats *simStats, bool *more)
{
	int qsize;
	int w, a, b, c, r, tr;
	if(clock == custStatus->getStartTime() + custStatus->getServiceTime() - 1)
	{
		w = custStatus->getStartTime() - custStatus->getCurrent_arrivalTime();
		custStatus->setWaitTime(w);

		r = custStatus->getWaitTime() + custStatus->getServiceTime();
		custStatus->setResponseTime(r);

		tr = simStats->getTotResponseTime() + custStatus->getResponseTime();
		simStats->setTotResponseTime(tr);

		c = custStatus->getCurrent_custNum();
		simStats->setTotCust(c);

		a = simStats->getTotServiceTime() + custStatus->getServiceTime();
		simStats->setTotServiceTime(a);

		b = simStats->getTotWaitTime() + custStatus->getWaitTime();
		simStats->setTotWaitTime(b);

		qsize = simQueue->getQueueSize();

		if(simStats->getMaxQueueSize() < qsize)
			simStats->setMaxQueueSize(qsize);
		*more = false;	
		printStatus(custStatus);
	}
	else 
		cout << "Task not complete\n";
	//cout << "Total Customers : \t" << simStats->getTotCust() << endl;

	return;
}

void printStatus(customerStatus *custStatus)
{
	cout << "=====================================\n";
	cout << "Customer Number Number : \t" << custStatus->getCurrent_custNum() << endl;
	cout << "Customer Arrival Time : \t: " << custStatus->getCurrent_arrivalTime() << endl;
	cout << "Customer Start Time : \t" << custStatus->getStartTime() << endl;
	cout << "Customer Service Time : \t" << custStatus->getServiceTime() << endl;
	cout << "Customer Wait Time : \t" << custStatus->getWaitTime() << endl;
	cout << "Customer Response Time : \t" << custStatus->getResponseTime() << endl;
	return;
}
void printStats(simulationStats *simStats)
{
	cout << fixed << setprecision(2) << showpoint;
	float r, s, w;
	s= simStats->getTotServiceTime() / float(simStats->getTotCust());
	w = simStats->getTotWaitTime() / float(simStats->getTotCust());
	r = simStats->getTotResponseTime() / float(simStats->getTotCust());
	cout << "===============================\n";
	cout << "Totoal Customers : \t" << simStats->getTotCust() << endl;
	cout << "Total Service Time : \t" << simStats->getTotServiceTime() << endl;
	cout << "Total Wait Time : \t" << simStats->getTotWaitTime() << endl;
	cout << "Total Response Time : \t" << simStats->getTotResponseTime() << endl;
	cout << "Average Service Time : \t" << s << endl;
	cout << "Average Wait Time : \t" << w << endl;
	cout << "Average Response Time : \t" << r << endl;
	cout << "Max Queue Size : \t" << simStats->getMaxQueueSize() << endl;
	cout << "===============================\n";
	return;
}
