#include "util.h"


unsigned int atoi(char *str)
{
	char *p = str;
	unsigned int ret = 0;

	while ('\0' != p && *p != '\0')
	{
		if ('0' <= *p && *p <= '9')
		{
			ret = ret * 10 + (*p - '0');
		}
		else
		{
			return 0;
		}
		p++;
	}

	return ret;
}



const char *get_fullpath(char *fullpath, int size, struct dentry *pd)
{
	char *namebuf1 = NULL;
	char *namebuf2 = NULL;
	struct dentry *pdentry;
	int i = 0;

	namebuf1 = (char *)kmalloc(MAX_PATH, GFP_KERNEL);
	namebuf2 = (char *)kmalloc(MAX_PATH, GFP_KERNEL);

	if (!pd || !fullpath || !namebuf1 || !namebuf2)
	{
		goto out;
	}

	pdentry = pd;
	memset(namebuf2, 0, sizeof(namebuf2));

	while (pdentry)
	{
		if (i++ > 30) {
			DLog("i > 30"); 
			break;
		}
		if (0 == strcmp(pdentry->d_iname, "/"))
		{
			strcpy(namebuf1, "/");
			strcat(namebuf1, namebuf2);
			strcpy(namebuf2, namebuf1);
			break;
		}
		strcpy(namebuf1, pdentry->d_iname);
		strcat(namebuf1, "/");
		strcat(namebuf1, namebuf2);
		strcpy(namebuf2, namebuf1);
		if (pdentry == pdentry->d_parent)
		{
			break;
		}
		pdentry = pdentry->d_parent;
	}

	strcpy(fullpath, namebuf2);

out:
	AvoidNull(kfree, namebuf1);
	AvoidNull(kfree, namebuf2);

	return (const char *)fullpath;
}

const char *get_path(char *fullpath, int size, unsigned int fd)
{
	struct file * pfile;
	struct dentry *pdentry;

	pfile = fget(fd);
	if (!pfile)
	{
		goto out;
	}

	pdentry = pfile->f_path.dentry;
	fput(pfile);
	get_fullpath(fullpath, size, pdentry);

out:
	return fullpath;
}

