/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*

 * Author: Reza Poorzare <reza.poorzare@upc.edu>
 *
 * Anna Calveras <anna.calveras@upc.edu>, Supervisor

 */

#include "Tcp-fbtcp.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Tcpfbtcp");
NS_OBJECT_ENSURE_REGISTERED (Tcpfbtcp);

TypeId
Tcpfbtcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Tcpfbtcp")
    .SetParent<TcpNewReno> ()
    .AddConstructor<Tcpfbtcp> ()
    .SetGroupName ("Internet")

  ;
  return tid;
}

Tcpfbtcp::Tcpfbtcp (void)
  : TcpNewReno (),
    m_baseRtt (Time::Max ()),
    m_minRtt (Time::Max ()),
    m_cntRtt (0),
    m_doingfbtcpNow (true),
    m_begSndNxt (0)
{
  NS_LOG_FUNCTION (this);
}

Tcpfbtcp::Tcpfbtcp (const Tcpfbtcp& sock)
  : TcpNewReno (sock),

    m_baseRtt (sock.m_baseRtt),
    m_minRtt (sock.m_minRtt),
    m_cntRtt (sock.m_cntRtt),
    m_doingfbtcpNow (true),
    m_begSndNxt (0)
{
  NS_LOG_FUNCTION (this);
}

Tcpfbtcp::~Tcpfbtcp (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<TcpCongestionOps>
Tcpfbtcp::Fork (void)
{
  return CopyObject<Tcpfbtcp> (this);
}


uint32_t congestionCounter=0;
uint32_t fixCounter=0;

static double currentThroughput;
uint32_t maxTempCwnd=0;
uint32_t convergenceCounter=0;
uint32_t divergenceCounter=0;


void
Tcpfbtcp::setThreshold(double th){


currentThroughput=th;
}



void
Tcpfbtcp::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                     const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);

  if (rtt.IsZero ())
    {
      return;
    }

  m_minRtt = std::min (m_minRtt, rtt);
  NS_LOG_DEBUG ("Updated m_minRtt = " << m_minRtt);

  m_baseRtt = std::min (m_baseRtt, rtt);
  NS_LOG_DEBUG ("Updated m_baseRtt = " << m_baseRtt);

  // Update RTT counter
  m_cntRtt++;
  NS_LOG_DEBUG ("Updated m_cntRtt = " << m_cntRtt);
}

void
Tcpfbtcp::Enablefbtcp (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);

  m_doingfbtcpNow = true;
  m_begSndNxt = tcb->m_nextTxSequence;
  m_cntRtt = 0;
  m_minRtt = Time::Max ();
}

void
Tcpfbtcp::Disablefbtcp ()
{
  NS_LOG_FUNCTION (this);

  m_doingfbtcpNow = false;
}

void
Tcpfbtcp::CongestionStateSet (Ptr<TcpSocketState> tcb,
                              const TcpSocketState::TcpCongState_t newState)
{
  NS_LOG_FUNCTION (this << tcb << newState);
  if (newState == TcpSocketState::CA_OPEN)
    {
      Enablefbtcp (tcb);
    }
  else
    {
      Disablefbtcp ();
    }
}

void
Tcpfbtcp::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (!m_doingfbtcpNow)
    {
      // If fbtcp is not on, we follow NewReno algorithm
      NS_LOG_LOGIC ("fbtcp is not turned on, we follow NewReno algorithm.");
      TcpNewReno::IncreaseWindow (tcb, segmentsAcked);
      return;
    }

  if (tcb->m_lastAckedSeq >= m_begSndNxt)
    { // A fbtcp cycle has finished, we do fbtcp cwnd adjustment every RTT.

      NS_LOG_LOGIC ("A fbtcp cycle has finished, we adjust cwnd once per RTT.");

      // Save the current right edge for next fbtcp cycle
      m_begSndNxt = tcb->m_nextTxSequence;

      /*
       * We perform fbtcp calculations only if we got enough RTT samples to
       * insure that at least 1 of those samples wasn't from a delayed ACK.
       */
      if (m_cntRtt <= 2)
        {  // We do not have enough RTT samples, so we should behave like Reno
          NS_LOG_LOGIC ("We do not have enough RTT samples to do fbtcp, so we behave like NewReno.");
          TcpNewReno::IncreaseWindow (tcb, segmentsAcked);
        }
      else
        {
          NS_LOG_LOGIC ("We have enough RTT samples to perform fbtcp calculations");
          /*
           * We have enough RTT samples to perform fbtcp algorithm.
           * Now we need to determine if cwnd should be increased or decreased
 
           */

          uint32_t diff;
          double targetCwnd;
          double segCwnd = tcb->GetCwndInSegments ();

          double targetcwndvscwnd;



          /*
           * little trick:
           * desidered throughput is currentCwnd * baseRtt
           */
           double tmp = m_baseRtt.GetSeconds () / m_minRtt.GetSeconds ();
        

  
          targetCwnd = static_cast<uint32_t> (segCwnd * tmp);
          NS_LOG_DEBUG ("Calculated targetCwnd = " << targetCwnd);
          NS_ASSERT (segCwnd >= targetCwnd);          


          diff = segCwnd - targetCwnd;
          targetcwndvscwnd=  (targetCwnd/segCwnd);

                
//Displaying the parameters.
        std::cout << "Minimum rtt is = " <<   m_baseRtt.GetSeconds () << std::endl;        
        std::cout << "rtt is= " <<   m_minRtt.GetSeconds () << std::endl;        
        std::cout << "basertt/rtt= " <<   tmp << std::endl;        

      
        std::cout << "Current cwnd= " <<   segCwnd << std::endl;        
        std::cout << "Targeted cwnd (The optimal cwnd)= " <<   targetCwnd << std::endl;    

        std::cout << "targetcwnd/cwnd= " <<   targetcwndvscwnd << std::endl;    


    

        std::cout << "diff (Current cwnd - The optimal cwnd)= " <<   diff << std::endl;  

        //std::cout << "Current cwnd in bytes " <<   tcb->m_cWnd << std::endl; 

        //std::cout << "Current ssthresh in bytes " <<   tcb->m_ssThresh << std::endl; 




          NS_LOG_DEBUG ("Calculated diff = " << diff);




//Slow Start.
           if (segCwnd  < 900)
//< tcb->m_ssThresh)
            {     // Slow start mode

std::cout << "In Slow Start" << std::endl;
              NS_LOG_LOGIC ("We are in slow start.");



std::cout<< "diff " << diff << std::endl;

                 segCwnd = 2 * segCwnd ;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;



            }
          else
            {     
            



          double temporaryCwnd;

std::cout << "Current passed throughput is: "<< currentThroughput << std::endl; 


       temporaryCwnd=(( (currentThroughput * m_baseRtt.GetSeconds ()) /8)/ tcb->m_segmentSize)* 1.05; //5 percent more than that.

std::cout << "Current temporaryCwnd: "<< temporaryCwnd << std::endl; 





if (temporaryCwnd > maxTempCwnd){
maxTempCwnd= int(temporaryCwnd);
}
else {
maxTempCwnd= int(maxTempCwnd);
}

std::cout << "Current maxTempCwnd: "<< maxTempCwnd << std::endl; 


std::cout << "congestionCounter is: " <<   congestionCounter << std::endl;
std::cout << "fixCounter is: " <<   fixCounter << std::endl;









if (segCwnd <= maxTempCwnd)
{
//Convergence phase has been initiated.
convergenceCounter+=1;
//std::cout << "Convergen counter is: " <<   convergenceCounter << std::endl;
//Harche targetcwndvscwnd yani we are moving too fast.

 if  ((0.98 <= tmp) && (tmp <= 1)  && (diff<=10)) {


                  segCwnd=segCwnd + 120;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "11111" << std::endl;   
congestionCounter=0;     
fixCounter=0;
}



else if  ((0.98 <= tmp) && (tmp <= 1)  && (diff >10) && (diff <= 15)) {


                  segCwnd=segCwnd + 100;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "22222" << std::endl;       
congestionCounter=0; 
fixCounter=0;
}

else if  ((0.98 <= tmp) && (tmp <= 1)  && (diff > 15) && (diff <= 20)) {


                  segCwnd=segCwnd + 70;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "33333" << std::endl;  
congestionCounter=0;      
fixCounter=0;
}

else if  ((0.98 <= tmp) && (tmp <= 1)  && (diff > 20) && (diff <= 30)) {


                  segCwnd=segCwnd + 60;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "44444" << std::endl;  
congestionCounter=0;      
fixCounter=0;
}


 else if  ((0.98 <= tmp) && (tmp <= 1)  && (diff > 30)) {


                  segCwnd=segCwnd + 50;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "55555 " << std::endl; 
congestionCounter=0;       
fixCounter=0;
}

else if  ((0.95 <= tmp) && (tmp <= 0.98) && ( targetcwndvscwnd >= 0.95) ){



                  segCwnd=segCwnd + 40;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "66666 " << std::endl;        

congestionCounter=0;
fixCounter=0;
}

else if  ((0.95 <= tmp) && (tmp <= 0.98) && ( targetcwndvscwnd < 0.95) ){



                  segCwnd=segCwnd + 30;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "77777 " << std::endl;        
congestionCounter=0;
fixCounter=0;
}




else if ((0.7 <= tmp) && (tmp < 0.98) && ( targetcwndvscwnd >= 0.95) ) {


                  segCwnd=segCwnd + 25;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "88888 " << std::endl;  
congestionCounter=0; 
fixCounter=0;    
}

else if ((0.7 <= tmp) && (tmp < 0.98) && ( targetcwndvscwnd < 0.95) ) {


                  segCwnd=segCwnd + 20;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;


        std::cout << "99999 " << std::endl;        

congestionCounter=0;
fixCounter=0;
}


else if  ((0.3 <= tmp) && (tmp < 0.7)){ 

congestionCounter=0;
fixCounter+=1;

//For 30 rtts.
if (fixCounter <= 30 ){

                  segCwnd=segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "10 10 10 10 10-a " << std::endl;  
                        }
else { 

                  segCwnd= 0.9 * segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "10 10 10 10 10-b " << std::endl;
fixCounter=0;
maxTempCwnd= 0.9 * maxTempCwnd;

}
     
}

else if  ((0.05 <= tmp) &&  (tmp < 0.3) && ( targetcwndvscwnd >= 0.95)) {

congestionCounter+=1;   
fixCounter=0;
//For 3 rtts.
if (congestionCounter < 3){
                  segCwnd=segCwnd - 10;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "11 11 11 11 11-a " << std::endl;
                             }
else{
                  segCwnd=segCwnd - 20;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "11 11 11 11 11-b" << std::endl;
congestionCounter=0;
fixCounter=0;

}    

     
}


else if  ((0.05 <= tmp) &&  (tmp < 0.3) && ( targetcwndvscwnd < 0.95)) {


fixCounter=0;
congestionCounter+=1;      
if (congestionCounter < 3){
                  segCwnd=segCwnd - 25;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "12 12 12 12 12-a " << std::endl; 
                            }
else{
                  segCwnd=segCwnd - 50;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "12 12 12 12 12-b" << std::endl; 
congestionCounter=0;
fixCounter=0;

} 

 

}



else if ((0.0 <= tmp) && (tmp < 0.05) && (targetcwndvscwnd >= 0.95)) {

fixCounter=0;
congestionCounter+=1;      
if (congestionCounter < 3){
                  segCwnd= 0.75 *segCwnd ;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "13 13 13 13 13-a " << std::endl; 
                                }

else{
                  segCwnd= segCwnd/2 ;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "13 13 13 13 13-b " << std::endl;
congestionCounter=0;
fixCounter=0;
}

}

else if ((0.0 <= tmp) && (tmp < 0.05) && (targetcwndvscwnd < 0.95)) {

fixCounter=0;
congestionCounter+=1;      
if (congestionCounter < 3){

                  segCwnd= 0.4 * segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "14 14 14 14 14-a " << std::endl;    
                            }
else{
                  segCwnd=  segCwnd/3;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "14 14 14 14 14-b" << std::endl;  
congestionCounter=0;
fixCounter=0;
}  

                                        } 
else {
fixCounter=0;
congestionCounter=0;
                  segCwnd=segCwnd;
        std::cout << "None of the conditions-a" << std::endl;        
}

 
} 










else
{
//Divergence Phase has been initiated.
divergenceCounter+=1;
//std::cout << "Divergence counter is: " << divergenceCounter <<std::endl;

std:: cout << "ENTERED THE DIVERGENCE PHASE. "<< std::endl;
  if  ((0.99 <= tmp) && (tmp <= 1) ) {

fixCounter=0;
congestionCounter=0;
                  segCwnd=segCwnd + 1;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "D-11111 " << std::endl;        

}



else if ((0.7 <= tmp) && (tmp < 0.99) && (targetcwndvscwnd >= 0.95) ) {

fixCounter=0;
congestionCounter=0;
                  segCwnd=segCwnd + 1;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "D-22222 " << std::endl;        
}


else if ((0.7 <= tmp) && (tmp < 0.99) && (targetcwndvscwnd < 0.95)  ) {

fixCounter+=1;
congestionCounter=0;

if (fixCounter <= 30 ){
                  segCwnd=segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "D-33333-a " << std::endl;   
                        }
else {

                  segCwnd= 0.9 * segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "D-33333-b " << std::endl;
fixCounter=0;
maxTempCwnd= 0.9 * maxTempCwnd;
}

     
}



else if  ((0.3 <= tmp) && (tmp < 0.7)){ 

fixCounter+=1;
congestionCounter=0;

if (fixCounter <= 30 ){
                  segCwnd=segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "D-44444-a " << std::endl;  
                        }

else {
                  segCwnd= 0.9 * segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;

        std::cout << "D-44444-b " << std::endl;
fixCounter=0;
maxTempCwnd= 0.9 * maxTempCwnd;
}

      
}



else if  ((0.05 <= tmp) &&  (tmp < 0.3) ) {

fixCounter=0;
congestionCounter+=1;      
if(congestionCounter < 3){
                  segCwnd=0.75 * segCwnd;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "D-55555-a " << std::endl; 
                
                        }
else{

                  segCwnd=segCwnd/2;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "D-55555-b " << std::endl; 
congestionCounter=0;
fixCounter=0;
}       
}



else if ((0.0 <= tmp) && (tmp < 0.05)) {

fixCounter=0;
congestionCounter+=1;      
if(congestionCounter < 3){
                  segCwnd=segCwnd / 2;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "D-66666-a " << std::endl; 

                              }

else{
                  segCwnd=segCwnd / 4;
                  tcb->m_cWnd = segCwnd * tcb->m_segmentSize;
                  tcb->m_ssThresh = GetSsThresh (tcb, 0);
        std::cout << "D-666666-b " << std::endl;       
congestionCounter=0;
fixCounter=0;
}


                                        } 
else {
fixCounter=0;
congestionCounter=0;
                  segCwnd=segCwnd;
        std::cout << "None of the conditions-b" << std::endl;        
}
}













}





std:: cout << "Convergence counter is: " << convergenceCounter << "   and divergence counter is: " << divergenceCounter<< std::endl;

          tcb->m_ssThresh = std::max (tcb->m_ssThresh, 3 * tcb->m_cWnd / 4);
          NS_LOG_DEBUG ("Updated ssThresh = " << tcb->m_ssThresh);
        } 



      // Reset cntRtt & minRtt every RTT
      m_cntRtt = 0;
      m_minRtt = Time::Max ();


    } 





}

std::string
Tcpfbtcp::GetName () const
{
  return "Tcpfbtcp";
}


uint32_t
Tcpfbtcp::GetSsThresh (Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);


 return std::max (std::min (tcb->m_ssThresh.Get (), tcb->m_cWnd.Get () - tcb->m_segmentSize), 2 * tcb->m_segmentSize);
}

} // namespace ns3
