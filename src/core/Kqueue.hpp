#ifndef KQUEUE_HPP
# define KQUEUE_HPP

# include "type.hpp"

# include <sys/event.h>
typedef struct kevent event_t;

# define TIMEOUT_SEC 120
# define EVENT_POOL 100000

class Kqueue {

	public:
		Kqueue();
		virtual ~Kqueue();

		int				fd() const;
		const event_t&	que(const size_t&) const;

		int				renew();
		void			set(uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void*);

		void*			castUdata(const int&) const;
		int				castUdata(void*) const;
		
	private:	
		int				_fd;
		vec<event_t>	_que;
		struct timespec	_timeout;
};

#endif

/*
	KQUEUE(2)                     System Calls Manual                    KQUEUE(2)

	NAME
		kqueue, kevent, kevent64 and kevent_qos – kernel event notification
		mechanism

	LIBRARY
		Standard C Library (libc, -lc)

	SYNOPSIS
		#include <sys/types.h>
		#include <sys/event.h>
		#include <sys/time.h>

		int
		kqueue(void);

		int
		kevent(int kq, const struct kevent *changelist, int nchanges,
			struct kevent *eventlist, int nevents,
			const struct timespec *timeout);

		int
		kevent64(int kq, const struct kevent64_s *changelist, int nchanges,
			struct kevent64_s *eventlist, int nevents, unsigned int flags,
			const struct timespec *timeout);

		int
		kevent_qos(int kq, const struct kevent_qos_s *changelist, int nchanges,
			struct kevent_qos_s *eventlist, int nevents, void *data_out,
			size_t *data_available, unsigned int flags);

		EV_SET(&kev, ident, filter, flags, fflags, data, udata);

		EV_SET64(&kev, ident, filter, flags, fflags, data, udata, ext[0],
			ext[1]);

		EV_SET_QOS(&kev, ident, filter, flags, qos, udata, fflags, xflags, data,
			ext[0], ext[1], ext[2], ext[3]);

	DESCRIPTION
		The kqueue() system call allocates a kqueue file descriptor.  This file
		descriptor provides a generic method of notifying the user when a kernel
		event (kevent) happens or a condition holds, based on the results of
		small pieces of kernel code termed filters.

		A kevent is identified by an (ident, filter, and optional udata value)
		tuple.  It specifies the interesting conditions to be notified about for
		that tuple. An (ident, filter, and optional udata value) tuple can only
		appear once in a given kqueue.  Subsequent attempts to register the same
		tuple for a given kqueue will result in the replacement of the conditions
		being watched, not an addition.  Whether the udata value is considered as
		part of the tuple is controlled by the EV_UDATA_SPECIFIC flag on the
		kevent.

		The filter identified in a kevent is executed upon the initial
		registration of that event in order to detect whether a preexisting
		condition is present, and is also executed whenever an event is passed to
		the filter for evaluation.  If the filter determines that the condition
		should be reported, then the kevent is placed on the kqueue for the user
		to retrieve.

		The filter is also run when the user attempts to retrieve the kevent from
		the kqueue.  If the filter indicates that the condition that triggered
		the event no longer holds, the kevent is removed from the kqueue and is
		not returned.

		Multiple events which trigger the filter do not result in multiple
		kevents being placed on the kqueue; instead, the filter will aggregate
		the events into a single struct kevent.  Calling close() on a file
		descriptor will remove any kevents that reference the descriptor.

		The kqueue() system call creates a new kernel event queue and returns a
		descriptor.  The queue is not inherited by a child created with fork(2).

		The kevent,() kevent64() and kevent_qos() system calls are used to
		register events with the queue, and return any pending events to the
		user.  The changelist argument is a pointer to an array of kevent,
		kevent64_s or kevent_qos_s structures, as defined in ⟨sys/event.h⟩.  All
		changes contained in the changelist are applied before any pending events
		are read from the queue.  The nchanges argument gives the size of
		changelist.

		The eventlist argument is a pointer to an array of out kevent, kevent64_s
		or kevent_qos_s structures.  The nevents argument determines the size of
		eventlist.

		The data_out argument provides space for extra out data provided by
		specific filters.  The data_available argument's contents specified the
		space available in the data pool on input, and contains the amount still
		remaining on output.  If the KEVENT_FLAG_STACK_DATA flag is specified on
		the system call, the data is allocated from the pool in stack order
		instead of typical heap order.

		If timeout is a non-NULL pointer, it specifies a maximum interval to wait
		for an event, which will be interpreted as a struct timespec.  If timeout
		is a NULL pointer, both kevent() and kevent64() wait indefinitely.  To
		effect a poll, the flags argument to kevent64() or kevent_qos() can
		include the KEVENT_FLAG_IMMEDIATE value to indicate an immediate timeout.
		Alternatively, the timeout argument should be non-NULL, pointing to a
		zero-valued timespec structure.  The same array may be used for the
		changelist and eventlist.

		The EV_SET() macro is provided for ease of initializing a kevent
		structure. Similarly, EV_SET64() initializes a kevent64_s structure and
		EV_SET_QOS() initializes a kevent_qos_s structure.

		The kevent, kevent64_s and kevent_qos_s structures are defined as:

		struct kevent {
				uintptr_t       ident;          identifier for this event
				int16_t         filter;         filter for event
				uint16_t        flags;          general flags
				uint32_t        fflags;         filter-specific flags
				intptr_t        data;           filter-specific data
				void            *udata;         opaque user data identifier
		};

		struct kevent64_s {
				uint64_t        ident;          identifier for this event
				int16_t         filter;         filter for event
				uint16_t        flags;          general flags
				uint32_t        fflags;         filter-specific flags
				int64_t         data;           filter-specific data
				uint64_t        udata;          opaque user data identifier
				uint64_t        ext[2];         filter-specific extensions
		};

		struct kevent_qos_s {
				uint64_t        ident;          identifier for this event
				int16_t         filter;         filter for event
				uint16_t        flags;          general flags
				uint32_t        qos;            quality of service when servicing event
				uint64_t        udata;          opaque user data identifier
				uint32_t        fflags;         filter-specific flags
				uint32_t        xflags;         extra filter-specific flags
				int64_t         data;           filter-specific data
				uint64_t        ext[4];         filter-specific extensions
		};

		----

		The fields of struct kevent, struct kevent64_s and struct kevent_qos_s
		are:

		ident      Value used to identify the source of the event.  The exact
					interpretation is determined by the attached filter, but often
					is a file descriptor.

		filter     Identifies the kernel filter used to process this event.  The
					pre-defined system filters are described below.

		flags      Actions to perform on the event.

		fflags     Filter-specific flags.

		data       Filter-specific data value.

		udata      Opaque user-defined value passed through the kernel unchanged.
					It can optionally be part of the uniquing decision of the
					kevent system

		In addition, struct kevent64_s contains:

		ext[2]     This field stores extensions for the event's filter. What type
					of extension depends on what type of filter is being used.

		In addition, struct kevent_qos_s contains:

		xflags     Extra filter-specific flags.

		ext[4]     The QoS variant provides twice as many extension values for
					filter-specific uses.

		----

		The flags field can contain the following values:

		EV_ADD         Adds the event to the kqueue.  Re-adding an existing event
						will modify the parameters of the original event, and not
						result in a duplicate entry.  Adding an event
						automatically enables it, unless overridden by the
						EV_DISABLE flag.

		EV_ENABLE      Permit kevent,() kevent64() and kevent_qos() to return the
						event if it is triggered.

		EV_DISABLE     Disable the event so kevent,() kevent64() and kevent_qos()
						will not return it.  The filter itself is not disabled.

		EV_DELETE      Removes the event from the kqueue.  Events which are
						attached to file descriptors are automatically deleted on
						the last close of the descriptor.

		EV_RECEIPT     This flag is useful for making bulk changes to a kqueue
						without draining any pending events. When passed as input,
						it forces EV_ERROR to always be returned.  When a filter
						is successfully added, the data field will be zero.

		EV_ONESHOT     Causes the event to return only the first occurrence of
						the filter being triggered.  After the user retrieves the
						event from the kqueue, it is deleted.

		EV_CLEAR       After the event is retrieved by the user, its state is
						reset.  This is useful for filters which report state
						transitions instead of the current state.  Note that some
						filters may automatically set this flag internally.

		EV_EOF         Filters may set this flag to indicate filter-specific EOF
						condition.

		EV_OOBAND      Read filter on socket may set this flag to indicate the
						presence of out of band data on the descriptor.

		EV_ERROR       See RETURN VALUES below.

		----

		The predefined system filters are listed below.  Arguments may be passed
		to and from the filter via the data, fflags and optionally xflags fields
		in the kevent, kevent64_s or kevent_qos_s structure.

		EVFILT_READ      Takes a file descriptor as the identifier, and returns
						whenever there is data available to read.  The behavior
						of the filter is slightly different depending on the
						descriptor type.

						Sockets
							Sockets which have previously been passed to
							listen() return when there is an incoming connection
							pending.  data contains the size of the listen
							backlog.

							Other socket descriptors return when there is data
							to be read, subject to the SO_RCVLOWAT value of the
							socket buffer.  This may be overridden with a per-
							filter low water mark at the time the filter is
							added by setting the NOTE_LOWAT flag in fflags, and
							specifying the new low water mark in data.  The
							derived per filter low water mark value is, however,
							bounded by socket receive buffer's high and low
							water mark values.  On return, data contains the
							number of bytes of protocol data available to read.

							The presence of EV_OOBAND in flags, indicates the
							presence of out of band data on the socket data
							equal to the potential number of OOB bytes availble
							to read.

							If the read direction of the socket has shutdown,
							then the filter also sets EV_EOF in flags, and
							returns the socket error (if any) in fflags.  It is
							possible for EOF to be returned (indicating the
							connection is gone) while there is still data
							pending in the socket buffer.

						Vnodes
							Returns when the file pointer is not at the end of
							file.  data contains the offset from current
							position to end of file, and may be negative.

						Fifos, Pipes
							Returns when there is data to read; data contains
							the number of bytes available.

							When the last writer disconnects, the filter will
							set EV_EOF in flags.  This may be cleared by passing
							in EV_CLEAR, at which point the filter will resume
							waiting for data to become available before
							returning.

						Device nodes
							Returns when there is data to read from the device;
							data contains the number of bytes available.  If the
							device does not support returning number of bytes,
							it will not allow the filter to be attached.
							However, if the NOTE_LOWAT flag is specified and the
							data field contains 1 on input, those devices will
							attach - but cannot be relied upon to provide an
							accurate count of bytes to be read on output.

		EVFILT_EXCEPT    Takes a descriptor as the identifier, and returns
						whenever one of the specified exceptional conditions has
						occurred on the descriptor. Conditions are specified in
						fflags.  Currently, this filter can be used to monitor
						the arrival of out-of-band data on a socket descriptor
						using the filter flag NOTE_OOB.

						If the read direction of the socket has shutdown, then
						the filter also sets EV_EOF in flags, and returns the
						socket error (if any) in fflags.

		EVFILT_WRITE     Takes a file descriptor as the identifier, and returns
						whenever it is possible to write to the descriptor.  For
						sockets, pipes and fifos, data will contain the amount
						of space remaining in the write buffer.  The filter will
						set EV_EOF when the reader disconnects, and for the fifo
						case, this may be cleared by use of EV_CLEAR.  Note that
						this filter is not supported for vnodes.

						For sockets, the low water mark and socket error
						handling is identical to the EVFILT_READ case.

		EVFILT_AIO       This filter is currently unsupported.

		EVFILT_VNODE     Takes a file descriptor as the identifier and the events
						to watch for in fflags, and returns when one or more of
						the requested events occurs on the descriptor.  The
						events to monitor are:

						NOTE_DELETE    The unlink() system call was called on
										the file referenced by the descriptor.

						NOTE_WRITE     A write occurred on the file referenced
										by the descriptor.

						NOTE_EXTEND    The file referenced by the descriptor was
										extended.

						NOTE_ATTRIB    The file referenced by the descriptor had
										its attributes changed.

						NOTE_LINK      The link count on the file changed.

						NOTE_RENAME    The file referenced by the descriptor was
										renamed.

						NOTE_REVOKE    Access to the file was revoked via
										revoke(2) or the underlying fileystem was
										unmounted.

						NOTE_FUNLOCK   The file was unlocked by calling flock(2)
										or close(2)

						NOTE_LEASE_DOWNGRADE
										A lease break to downgrade the lease to
										read lease is requested on the file
										referenced by the descriptor.

						NOTE_LEASE_RELEASE
										A lease break to release the lease is
										requested on the file or directory
										referenced by the descriptor.

						On return, fflags contains the filter-specific flags
						which are associated with the triggered events seen by
						this filter.

		EVFILT_PROC      Takes the process ID to monitor as the identifier and
						the events to watch for in fflags, and returns when the
						process performs one or more of the requested events.
						If a process can normally see another process, it can
						attach an event to it.  The events to monitor are:

						NOTE_EXIT    The process has exited.

						NOTE_EXITSTATUS
									The process has exited and its exit status
									is in filter specific data. Valid only on
									child processes and to be used along with
									NOTE_EXIT.

						NOTE_FORK    The process created a child process via
									fork(2) or similar call.

						NOTE_EXEC    The process executed a new process via
									execve(2) or similar call.

						NOTE_SIGNAL  The process was sent a signal. Status can
									be checked via waitpid(2) or similar call.

						NOTE_REAP    The process was reaped by the parent via
									wait(2) or similar call. Deprecated, use
									NOTE_EXIT.

						On return, fflags contains the events which triggered
						the filter.

		EVFILT_SIGNAL    Takes the signal number to monitor as the identifier and
						returns when the given signal is generated for the
						process.  This coexists with the signal() and
						sigaction() facilities, and has a lower precedence.
						Only signals sent to the process, not to a particular
						thread, will trigger the filter. The filter will record
						all attempts to deliver a signal to a process, even if
						the signal has been marked as SIG_IGN.  Event
						notification happens before normal signal delivery
						processing.  data returns the number of times the signal
						has been generated since the last call to kevent().
						This filter automatically sets the EV_CLEAR flag
						internally.

		EVFILT_MACHPORT  Takes the name of a mach port, or port set, in ident and
						waits until a message is enqueued on the port or port
						set. When a message is detected, but not directly
						received by the kevent call, the name of the specific
						port where the message is enqueued is returned in data.
						If fflags contains MACH_RCV_MSG, the ext[0] and ext[1]
						flags are assumed to contain a pointer to the buffer
						where the message is to be received and the size of the
						receive buffer, respectively.  If MACH_RCV_MSG is
						specifed, yet the buffer size in ext[1] is zero, The
						space for the buffer may be carved out of the data_out
						area provided to kevent_qos() if there is enough space
						remaining there.

		EVFILT_TIMER     Establishes an interval timer identified by ident where
						data specifies the timeout period (in milliseconds).

						fflags can include one of the following flags to specify
						a different unit:

						NOTE_SECONDS   data is in seconds

						NOTE_USECONDS  data is in microseconds

						NOTE_NSECONDS  data is in nanoseconds

						NOTE_MACHTIME  data is in Mach absolute time units

						fflags can also include NOTE_ABSOLUTE, which establishes
						an EV_ONESHOT timer with an absolute deadline instead of
						an interval.  The absolute deadline is expressed in
						terms of gettimeofday(2).  With NOTE_MACHTIME, the
						deadline is expressed in terms of mach_absolute_time().

						The timer can be coalesced with other timers to save
						power. The following flags can be set in fflags to
						modify this behavior:

						NOTE_CRITICAL    override default power-saving
										techniques to more strictly respect the
										leeway value

						NOTE_BACKGROUND  apply more power-saving techniques to
										coalesce this timer with other timers

						NOTE_LEEWAY      ext[1] holds user-supplied slop in
										deadline for timer coalescing.

						The timer will be periodic unless EV_ONESHOT is
						specified.  On return, data contains the number of times
						the timeout has expired since the last arming or last
						delivery of the timer event.

						This filter automatically sets the EV_CLEAR flag.

		----

		In the ext[2] field of the kevent64_s struture, ext[0] is only used with
		the EVFILT_MACHPORT filter.  With other filters, ext[0] is passed through
		kevent64() much like udata.  ext[1] can always be used like udata.  For
		the use of ext[0], see the EVFILT_MACHPORT filter above.

	RETURN VALUES
		The kqueue() system call creates a new kernel event queue and returns a
		file descriptor.  If there was an error creating the kernel event queue,
		a value of -1 is returned and errno set.

		The kevent(), kevent64() and kevent_qos() system calls return the number
		of events placed in the eventlist, up to the value given by nevents.  If
		an error occurs while processing an element of the changelist and there
		is enough room in the eventlist, then the event will be placed in the
		eventlist with EV_ERROR set in flags and the system error in data.
		Otherwise, -1 will be returned, and errno will be set to indicate the
		error condition.  If the time limit expires, then kevent(), kevent64()
		and kevent_qos() return 0.
*/


/*

*/