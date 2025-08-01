/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jpelline <jpelline@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 13:42:43 by jpelline          #+#    #+#             */
/*   Updated: 2025/07/31 12:15:47 by jpelline         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	exit_simulation(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->number_of_philos)
	{
		pthread_join(table->philos[i].thread, NULL);
		i++;
	}
	i = 0;
	while (i < table->number_of_philos)
		pthread_mutex_destroy(&table->philos[i++].fork);
	pthread_mutex_destroy(&table->write_lock);
	pthread_mutex_destroy(&table->meal_lock);
	pthread_mutex_destroy(&table->dead_lock);
	return (0);
}

static int	all_philos_have_eaten(t_table *table)
{
	int	i;

	i = 0;
	pthread_mutex_lock(&table->meal_lock);
	while (i < table->number_of_philos)
	{
		if (table->philos[i].times_to_eat != 0 || table->number_of_philos == 1)
		{
			pthread_mutex_unlock(&table->meal_lock);
			return (1);
		}
		i++;
	}
	pthread_mutex_unlock(&table->meal_lock);
	return (0);
}

static int	handle_death(t_table *table, int i)
{
	ft_usleep(table->time_to_die, table);
	pthread_mutex_lock(&table->dead_lock);
	table->philos[i].has_died = true;
	table->death = true;
	pthread_mutex_unlock(&table->dead_lock);
	pthread_mutex_lock(&table->write_lock);
	printf(DEATH, table->time_to_die, table->philos[i].number);
	pthread_mutex_unlock(&table->write_lock);
	return (0);
}

static int	check_death_conditions(t_table *table, int i)
{
	pthread_mutex_lock(&table->meal_lock);
	if (table->time_to_eat + table->time_to_sleep > table->time_to_die
		&& table->philos[i].has_eaten == true)
	{
		pthread_mutex_unlock(&table->meal_lock);
		return (handle_death(table, i));
	}
	pthread_mutex_unlock(&table->meal_lock);
	check_time(&table->philos[i]);
	pthread_mutex_lock(&table->dead_lock);
	if (table->philos[i].has_died == true)
	{
		printf(DEATH, table->philos[i].death_time,
			table->philos[i].number);
		pthread_mutex_unlock(&table->dead_lock);
		return (0);
	}
	pthread_mutex_unlock(&table->dead_lock);
	return (1);
}

int	monitor(t_table *table)
{
	int	i;

	while (true)
	{
		i = 0;
		while (i < table->number_of_philos)
		{
			wait_for_start(&table->philos[i]);
			if (!check_death_conditions(table, i))
				return (exit_simulation(table));
			if (table->ac == 6 && !all_philos_have_eaten(table))
				return (exit_simulation(table));
			i++;
		}
		usleep(1000);
	}
}
