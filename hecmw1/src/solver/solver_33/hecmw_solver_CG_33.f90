!======================================================================!
!                                                                      !
!   Software Name : HEC-MW Library for PC-cluster                      !
!         Version : 2.6                                                !
!                                                                      !
!     Last Update : 2014/01/25                                         !
!        Category : Linear Solver                                      !
!                                                                      !
!            Written by Kengo Nakajima (Univ. of Tokyo)                !
!                       Kazuya Goto (PExProCS LLC)                     !
!                                                                      !
!     Contact address :  IIS,The University of Tokyo RSS21 project     !
!                                                                      !
!     "Structural Analysis System for General-purpose Coupling         !
!      Simulations Using High End Computing Middleware (HEC-MW)"       !
!                                                                      !
!======================================================================!

!C
!C***
!C*** module hecmw_solver_CG_33
!C***
!C
      module hecmw_solver_CG_33

      public :: hecmw_solve_CG_33
      private :: estimate_cond_num

      contains
!C
!C*** CG_33
!C
      subroutine hecmw_solve_CG_33( hecMESH,  hecMAT, ITER, RESID, ERROR, &
     &                              Tset, Tsol, Tcomm )

      use hecmw_util
      use m_hecmw_solve_error
      use m_hecmw_comm_f
      use hecmw_matrix_misc
      use hecmw_solver_misc
      use hecmw_solver_las_33
      use hecmw_solver_scaling_33
      use hecmw_precond_33
      use hecmw_jad_type

      implicit none

      type(hecmwST_local_mesh) :: hecMESH
      type(hecmwST_matrix) :: hecMAT
      integer(kind=kint ), intent(inout):: ITER, ERROR
      real   (kind=kreal), intent(inout):: RESID, Tset, Tsol, Tcomm

      integer(kind=kint ) :: N, NP, NDOF, NNDOF
      integer(kind=kint ) :: my_rank
      integer(kind=kint ) :: ITERlog, TIMElog
      real(kind=kreal), pointer :: B(:), X(:)

      real(kind=kreal), dimension(:,:), allocatable :: WW

      integer(kind=kint), parameter ::  R= 1
      integer(kind=kint), parameter ::  Z= 2
      integer(kind=kint), parameter ::  Q= 2
      integer(kind=kint), parameter ::  P= 3
      integer(kind=kint), parameter :: WK= 4

      integer(kind=kint ) :: MAXIT

! local variables
      real   (kind=kreal) :: TOL
      integer(kind=kint )::i
      real   (kind=kreal)::S_TIME,S1_TIME,E_TIME,E1_TIME, START_TIME, END_TIME
      real   (kind=kreal)::BNRM2
      real   (kind=kreal)::RHO,RHO1,BETA,C1,ALPHA,DNRM2
      real   (kind=kreal)::t_max,t_min,t_avg,t_sd
      integer(kind=kint) ::ESTCOND
      real   (kind=kreal), allocatable::D(:),E(:)
      real   (kind=kreal)::ALPHA1

      call hecmw_barrier(hecMESH)
      S_TIME= HECMW_WTIME()

!C===
!C +-------+
!C | INIT. |
!C +-------+
!C===
      N = hecMAT%N
      NP = hecMAT%NP
      NDOF = hecMAT%NDOF
      NNDOF = N * NDOF
      my_rank = hecMESH%my_rank
      X => hecMAT%X
      B => hecMAT%B

      ITERlog = hecmw_mat_get_iterlog( hecMAT )
      TIMElog = hecmw_mat_get_timelog( hecMAT )
      MAXIT  = hecmw_mat_get_iter( hecMAT )
      TOL   = hecmw_mat_get_resid( hecMAT )
      ESTCOND = hecmw_mat_get_estcond( hecMAT )

      ERROR = 0

      allocate (WW(NDOF*NP, 4))
      WW = 0.d0

!C
!C-- SCALING
      call hecmw_solver_scaling_fw_33(hecMESH, hecMAT, Tcomm)

      IF (hecmw_mat_get_usejad(hecMAT).ne.0) THEN
        call hecmw_JAD_INIT(hecMAT)
      ENDIF

      if (ESTCOND /= 0) then
        allocate(D(MAXIT),E(MAXIT-1))
      endif
!C===
!C +----------------------+
!C | SETUP PRECONDITIONER |
!C +----------------------+
!C===
      call hecmw_precond_33_setup(hecMAT)

!C===
!C +---------------------+
!C | {r0}= {b} - [A]{x0} |
!C +---------------------+
!C===
      call hecmw_matresid_33(hecMESH, hecMAT, X, B, WW(:,R), Tcomm)

!C-- compute ||{b}||
      call hecmw_InnerProduct_R(hecMESH, NDOF, B, B, BNRM2, Tcomm)
      if (BNRM2.eq.0.d0) then
        iter = 0
        MAXIT = 0
        RESID = 0.d0
        X = 0.d0
      endif

      call hecmw_barrier(hecMESH)
      E_TIME = HECMW_WTIME()
      call hecmw_time_statistics(hecMESH, E_TIME - S_TIME, &
           t_max, t_min, t_avg, t_sd)
      if (hecMESH%my_rank.eq.0) then
        write(*,*) 'Time solver setup'
        write(*,*) '  Max     :',t_max
        write(*,*) '  Min     :',t_min
        write(*,*) '  Avg     :',t_avg
        write(*,*) '  Std Dev :',t_sd
      endif
      Tset = Tset + t_max

      Tcomm = 0.d0
      S1_TIME = HECMW_WTIME()
!C
!C************************************************* Conjugate Gradient Iteration start
!C
      do iter = 1, MAXIT

!C===
!C +----------------+
!C | {z}= [Minv]{r} |
!C +----------------+
!C===
      call hecmw_precond_33_apply(hecMESH, hecMAT, WW(:,R), WW(:,Z), WW(:,WK), Tcomm)

!C===
!C +---------------+
!C | {RHO}= {r}{z} |
!C +---------------+
!C===
      call hecmw_InnerProduct_R(hecMESH, NDOF, WW(:,R), WW(:,Z), RHO, Tcomm)

!C===
!C +-----------------------------+
!C | {p} = {z} if      ITER=1    |
!C | BETA= RHO / RHO1  otherwise |
!C +-----------------------------+
!C===
      if ( ITER.eq.1 ) then
        do i = 1, NNDOF
          WW(i,P) = WW(i,Z)
        enddo
       else
         BETA = RHO / RHO1
         do i = 1, NNDOF
           WW(i,P) = WW(i,Z) + BETA*WW(i,P)
         enddo
      endif

!C===
!C +--------------+
!C | {q}= [A] {p} |
!C +--------------+
!C===
      call hecmw_matvec_33(hecMESH, hecMAT, WW(:,P), WW(:,Q), Tcomm)

!C===
!C +---------------------+
!C | ALPHA= RHO / {p}{q} |
!C +---------------------+
!C===
      call hecmw_InnerProduct_R(hecMESH, NDOF, WW(:,P), WW(:,Q), C1, Tcomm)

      ALPHA= RHO / C1

!C===
!C +----------------------+
!C | {x}= {x} + ALPHA*{p} |
!C | {r}= {r} - ALPHA*{q} |
!C +----------------------+
!C===
      do i = 1, NNDOF
         X(i)  = X(i)    + ALPHA * WW(i,P)
        WW(i,R)= WW(i,R) - ALPHA * WW(i,Q)
      enddo

      call hecmw_InnerProduct_R(hecMESH, NDOF, WW(:,R), WW(:,R), DNRM2, Tcomm)

      RESID= dsqrt(DNRM2/BNRM2)

!C##### ITERATION HISTORY
      if (my_rank.eq.0.and.ITERLog.eq.1) write (*,'(i7, 1pe16.6)') ITER, RESID
!C#####

      if (ESTCOND /= 0) then
        call estimate_cond_num(ITER, ALPHA, BETA, ALPHA1, D, E)
        ALPHA1 = ALPHA
      endif

      if ( RESID.le.TOL   ) exit
      if ( ITER .eq.MAXIT ) ERROR = -300

      RHO1 = RHO

      enddo
!C
!C************************************************* Conjugate Gradient Iteration end
!C
      call hecmw_solver_scaling_bk_33(hecMAT)
!C
!C-- INTERFACE data EXCHANGE
!C
      START_TIME= HECMW_WTIME()
      call hecmw_update_3_R (hecMESH, X, hecMAT%NP)
      END_TIME = HECMW_WTIME()
      Tcomm = Tcomm + END_TIME - START_TIME

      deallocate (WW)
      call hecmw_precond_33_clear(hecMAT)

      IF (hecmw_mat_get_usejad(hecMAT).ne.0) THEN
        call hecmw_JAD_FINALIZE()
      ENDIF

      if (ESTCOND /= 0) then
        deallocate(D, E)
      endif

      call hecmw_barrier(hecMESH)
      E1_TIME = HECMW_WTIME()
      Tsol = E1_TIME - S1_TIME

      end subroutine hecmw_solve_CG_33

      subroutine estimate_cond_num(ITER, ALPHA, BETA, ALPHA1, D, E)
      use hecmw_util
      implicit none
      integer(kind=kint), intent(in) :: ITER
      real(kind=kreal), intent(in) :: ALPHA, BETA, ALPHA1
      real(kind=kreal), intent(inout) :: D(:), E(:)
      character(len=1) :: JOBZ, RANGE
      ! character(len=1) :: COMPZ
      real(kind=kreal) :: VL, VU, ABSTOL, Z(1,1)
      integer(kind=kint) :: N, IL, IU, M, LDZ=1, ISUPPZ(1)
      integer(kind=kint) :: LWORK, LIWORK, INFO
      real(kind=kreal), allocatable :: W(:), WORK(:)
      integer(kind=kint), allocatable :: IWORK(:)
      real(kind=kreal), allocatable :: D1(:), E1(:)
      integer(kind=kint) :: i

      if (ITER == 1) then
        D(1) = 1.d0 / ALPHA
      else
        D(ITER) = 1.d0 / ALPHA + BETA / ALPHA1
        E(ITER-1) = sqrt(BETA) / ALPHA1
      endif

      ! copy D, E
      allocate(D1(ITER),E1(ITER))
      do i=1,ITER-1
        D1(i) = D(i)
        E1(i) = E(i)
      enddo
      D1(ITER) = D(ITER)


      !!
      !! dstegr version (faster than dsteqr)
      !!

      ! prepare arguments for calling dstegr
      JOBZ='N'
      RANGE='A'
      N=ITER
      allocate(W(ITER))
      ! estimate optimal LWORK and LIWORK
      LWORK=-1
      LIWORK=-1
      allocate(WORK(1),IWORK(1))
      call dstegr(JOBZ,RANGE,N,D1,E1,VL,VU,IL,IU,ABSTOL, &
           M,W,Z,LDZ,ISUPPZ,WORK,LWORK,IWORK,LIWORK,INFO)
      if (INFO /= 0) then
        write(*,*) 'ERROR: dstegr returned with INFO=',INFO
        return
      endif
      ! calculate eigenvalues
      LWORK=WORK(1)
      LIWORK=IWORK(1)
      deallocate(WORK,IWORK)
      allocate(WORK(LWORK),IWORK(LIWORK))
      call dstegr(JOBZ,RANGE,N,D1,E1,VL,VU,IL,IU,ABSTOL, &
           M,W,Z,LDZ,ISUPPZ,WORK,LWORK,IWORK,LIWORK,INFO)
      if (INFO /= 0) then
        write(*,*) 'ERROR: dstegr returned with INFO=',INFO
        return
      endif
      write(*,'("emin=",1pe13.6,", emax=",1pe13.6,", emax/emin=",1pe13.6)') &
           W(1),W(N),W(N)/W(1)
      deallocate(WORK,IWORK)
      deallocate(W)


      ! !!
      ! !! dsteqr version
      ! !!

      ! ! prepare arguments for calling dsteqr
      ! COMPZ='N'
      ! N=ITER
      ! allocate(WORK(1))
      ! ! calculate eigenvalues
      ! call dsteqr(COMPZ,N,D1,E1,Z,LDZ,WORK,INFO)
      ! if (INFO /= 0) then
      !   write(*,*) 'ERROR: dsteqr returned with INFO=',INFO
      !   return
      ! endif
      ! write(*,'("emin=",1pe13.6,", emax=",1pe13.6,", emax/emin=",1pe13.6)') &
      !      D1(1),D1(N),D1(N)/D1(1)
      ! deallocate(WORK)


      deallocate(D1,E1)

      end subroutine estimate_cond_num
      end module     hecmw_solver_CG_33
