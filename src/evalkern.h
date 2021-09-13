#ifndef EVALKERN_H
#define EVALKERN_H

#ifndef PRULE_CONTEXT
#define PRULE_CONTEXT(pcb)  (&((pcb).cs[(pcb).ssx]))
#define PERROR_CONTEXT(pcb) ((pcb).cs[(pcb).error_frame_ssx])
#define PCONTEXT(pcb)       ((pcb).cs[(pcb).ssx])
#endif

#ifndef AG_RUNNING_CODE_CODE
/* PCB.exit_flag values */
#define AG_RUNNING_CODE         0
#define AG_SUCCESS_CODE         1
#define AG_SYNTAX_ERROR_CODE    2
#define AG_REDUCTION_ERROR_CODE 3
#define AG_STACK_ERROR_CODE     4
#define AG_SEMANTIC_ERROR_CODE  5
#endif

void init_evalKernel(evalKernel_pcb_type& evalKernel_pcb);
void evalKernel(VariableDescriptor* variable, int& nVariables,ErrorRecord& errorRecord,evalKernel_pcb_type& evalKernel_pcb,CharStack& cs,ArgStack& as);
#endif
