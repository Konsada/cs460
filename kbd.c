int get_scode()
{
	int scode, value;
	/* Fetch the character from the keyboard hardware */
	scode = in_byte(KEYBD);
	value = in_byte(PORT_B);
	out_byte(PORT_B, value | KBIT);
	out_byte(PORT_B, value);
	return scode
}

int kbhandler(){
	int scode, c;

	KBD *p = &kbd;

	scode = get_scode();

	printf("scode=%x\n", scode);

	// IMPLEMENT "HOT KEY" HERE!!!

	if(scode & 0x80){ // key release:
	goto out;
	}
	// translate scan code to ASCII, using shift[ ] table if shifted;
	c = unshift[scode];

	if(p->data == KBLEN){
	printf("BKD buffer full\n");
	goto out;
	}

	p->buf[p->head++] = c;
	p->head %= KBLEN;

	p->data++;

	kwakeup(&p->data);

	out:
	out_byte(0x20, 0x20) // done with interrupt processing ***CRUCIAL***
}

int getc(){
	char c;
	lock();

	while (kbd.data==0){
	unlock();
	ksleep(&kbd.data);
	lock();
	}

	c = kbd.buf[kbd.tail++];
	kbd.tail %= KBLEN;
	kbd.data--;

	unlock();

	return c;
}