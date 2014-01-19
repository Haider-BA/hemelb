<?xml version="1.0"?>
<report>
	<configuration>
		<file>{{CONFIG}}</file>
		<steps>{{TOTAL_TIME_STEPS}}</steps>
		<resolution>
			<timestep>{{TIME_STEP_LENGTH}}</timestep>
		</resolution>
	</configuration>
	{{#BUILD}}
	<build>
		<revision>{{REVISION}}</revision>
		<steering>{{STEERING}}</steering>
		<streaklines>{{STREAKLINES}}</streaklines>
		<multimachine>{{MULTIMACHINE}}</multimachine>
		<type>{{TYPE}}</type>
		<optimisation>{{OPTIMISATION}}</optimisation>
                <use_sse3>{{USE_SSE3}}</use_sse3>
		<date>{{TIME}}</date>
		<reading_group>{{READING_GROUP_SIZE}}</reading_group>
		<lattice_type>{{LATTICE_TYPE}}</lattice_type>
		<kernel_type>{{KERNEL_TYPE}}</kernel_type>
		<wall_boundary_condition>{{WALL_BOUNDARY_CONDITION}}</wall_boundary_condition>
		<inlet_boundary_condition>{{INLET_BOUNDARY_CONDITION}}</inlet_boundary_condition>
		<outlet_boundary_condition>{{OUTLET_BOUNDARY_CONDITION}}</outlet_boundary_condition>
		<wall_inlet_boundary_condition>{{WALL_INLET_BOUNDARY_CONDITION}}</wall_inlet_boundary_condition>
		<wall_outlet_boundary_condition>{{WALL_OUTLET_BOUNDARY_CONDITION}}</wall_outlet_boundary_condition>
		<communications>
			<implementations>
				<pointpoint>{{POINTPOINT_IMPLEMENTATION}}</pointpoint>
				<alltoall>{{ALLTOALL_IMPLEMENTATION}}</alltoall>
				<gathers>{{GATHERS_IMPLEMENTATION}}</gathers>
			</implementations>
			<separated_concerns>{{SEPARATE_CONCERNS}}</separated_concerns>
		</communications>
	</build>
	{{/BUILD}}
	<nodes>
		<threads>{{THREADS}}</threads>
		<machines>{{MACHINES}}</machines>
		<depths>{{DEPTHS}}</depths>
	</nodes>
	<geometry>
		<sites>{{SITES}}</sites>
		<blocks>{{BLOCKS}}</blocks>
		<sites_per_block>{{SITESPERBLOCK}}</sites_per_block>
		{{#PROCESSOR}}
		<domain>
			<rank>{{RANK}}</rank><sites>{{SITES}}</sites>
		</domain>
		{{/PROCESSOR}}
	</geometry>
	<results>
		<images>{{IMAGES}}</images>
		<steps>
			<total>{{STEPS}}</total>
		</steps>
	</results>
	<checks>
		{{#DENSITIES}}
		<density_problem>
			<allowed>{{ALLOWED}}</allowed>
			<actual>{{ACTUAL}}</actual>
		</density_problem>
		{{/DENSITIES}}
		{{#UNSTABLE}}
			<stability_problem/>
		{{/UNSTABLE}}
		{{#SOLUTIONCONVERGED}}
		<convergence_achieved/>
        {{/SOLUTIONCONVERGED}}
		
	</checks>
	<timings>
		{{#TIMER}}
		<timer>
			<name>{{NAME}}</name>
			<local>{{LOCAL}}</local>
			<min>{{MIN}}</min>
			<mean>{{MEAN}}</mean>
			<max>{{MAX}}</max>
		</timer>
		{{/TIMER}}
	</timings>
</report>