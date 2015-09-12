static int atomik_errno;

int *__errno_location(void)
{
	return &atomik_errno;
}
