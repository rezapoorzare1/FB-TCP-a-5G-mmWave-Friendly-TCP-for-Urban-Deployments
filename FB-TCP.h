/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Author: Reza Poorzare <reza.poorzare@upc.edu>
 *
 * Anna Calveras <anna.calveras@upc.edu>, Supervisor
 */

#ifndef TCPfbtcp_H
#define TCPfbtcp_H

#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-recovery-ops.h"

namespace ns3 {

/**
The FB-TCP protocol's code for the following paper:

"FB-TCP: a 5G mmWave Friendly TCP for Urban Deployments"

 */

class Tcpfbtcp : public TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Create an unbound tcp socket.
   */
  Tcpfbtcp (void);




//For the calculation of the upper bound.
   void setThreshold (double th);



  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  Tcpfbtcp (const Tcpfbtcp& sock);
  virtual ~Tcpfbtcp (void);

  virtual std::string GetName () const;

  /**
   * \brief Compute RTTs needed to execute fbtcp algorithm
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   * \param rtt last RTT
   *
   */
  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                          const Time& rtt);

  /**
   * \brief Enable/disable fbtcp algorithm depending on the congestion state
   *
   * We only start a fbtcp cycle when we are in normal congestion state (CA_OPEN state).
   *
   * \param tcb internal congestion state
   * \param newState new congestion state to which the TCP is going to switch
   */
  virtual void CongestionStateSet (Ptr<TcpSocketState> tcb,
                                   const TcpSocketState::TcpCongState_t newState);

  /**
   * \brief Adjust cwnd following fbtcp mechanism.
   *
   * \param tcb internal congestion state
   * \param segmentsAcked count of segments ACKed
   */
  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  /**
   *
   * \param tcb internal congestion state
   * \param bytesInFlight bytes in flight
   *
   * \return the slow start threshold value
   */
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual Ptr<TcpCongestionOps> Fork ();

protected:
private:
  /**
   * \brief Enable fbtcp algorithm to start taking fbtcp samples
   *
   * fbtcp algorithm is enabled in the following situations:
   * 1. at the establishment of a connection
   * 2. after an RTO
   * 3. after fast recovery
   * 4. when an idle connection is restarted
   *
   * \param tcb internal congestion state
   */
  void Enablefbtcp (Ptr<TcpSocketState> tcb);

  /**
   * \brief Stop taking fbtcp samples
   */
  void Disablefbtcp ();

private:
  Time m_baseRtt;                    //!< Minimum of all fbtcp RTT measurements seen during the connection
  Time m_minRtt;                     //!< Minimum of all RTT measurements within the last RTT
  uint32_t m_cntRtt;                 //!< Number of RTT measurements during the last RTT
  bool m_doingfbtcpNow;              //!< If true, do fbtcp for this RTT
  SequenceNumber32 m_begSndNxt;      //!< Right edge during last RTT
};

} // namespace ns3

#endif // TCPfbtcp_H
