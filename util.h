#ifndef UTIL_H
#define UTIL_H 1

#define UNUSED(x) (void)(x)

extern void	die(char const *, ...);

extern char	*argv0;
#define ARGBEGIN for(argv0=(*argv),--argc,++argv;*argv&&argv[0][0]=='-'&& \
                 argv[0][1];--argc,++argv){char*_args,*_argt;UNUSED(_argt); \
                 _args= &argv[0][1];if(_args[0]=='-'&&!_args[1]){--argc, \
                 ++argv;break;}for(;_args[0];++_args){switch(_args[0])

#define ARGEND }}

#define ARGC() *_args

#define ARGF() (_argt=_args+1,_args=" ",(*_argt?_argt:argv[1]?(--argc, \
               *++argv):(char*)0))
#define EARGF(x) (_argt=_args+1,_args=" ",(*_argt?_argt:argv[1]?(--argc, \
                 *++argv):((x),abort(),(char*)0)))

#endif /* UTIL_H */
