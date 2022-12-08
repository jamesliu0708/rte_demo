execute_process(
    COMMAND bash -c "${CC} -E -undef -P -x assembler-with-cpp -ffreestanding -o ${RTE_OUTPUT}/.config ${RTE_CONFIG_TEMPLATE}"
)

execute_process(
    COMMAND bash -c 
)