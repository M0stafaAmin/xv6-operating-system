// Spinning ticket lock.

struct ticketlock
{
	int next_ticket; // next ticket number to be given
	int current_turn; // current ticket number being served
	struct proc *proc; // process currently holding the lock
};

