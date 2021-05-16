from django.db import models
from django.core.validators import MinValueValidator, MaxValueValidator

# validator range data
MIN=0
MAX=2

# Create your models here.
class Polls(models.Model):
	poll = models.AutoField(primary_key=True)
	poll_title = models.CharField(max_length=50)
	poll_text = models.CharField(max_length=200)
	start_date = models.DateField('start_date')
	end_date = models.DateField('end_date')

	def __str__(self):
		return self.poll_title

class Questions(models.Model):
	question = models.AutoField(primary_key=True)
	polls_parent = models.ForeignKey(Polls, on_delete=models.CASCADE)
	question_text = models.CharField(max_length=200)
	# for questions only one answer
	question_answer = models.CharField(max_length=200, default='')
	# 0: only one text answer
	# 1: "radio button"
	# 2: "checkbox"
	question_type = models.IntegerField(validators=[MinValueValidator(MIN), MaxValueValidator(MAX)])
	# True if user had answered text answer
	question_check = models.BooleanField(default=False)

	def __str__(self):
		return self.question_text

class Choices(models.Model):
	choice = models.AutoField(primary_key=True)
	question_parent = models.ForeignKey(Questions, related_name='choice_name', on_delete=models.CASCADE)
	choice_text = models.CharField(max_length=200)
	# True if user had selected this choice 
	choice_check = models.BooleanField(default=False)

	def __str__(self):
		return self.choice_text + ': ' + str(self.choice)

class Answers(models.Model):
	answer = models.AutoField(primary_key=True)
	user = models.IntegerField()
	answer_text = models.CharField(max_length=200)
	choice_parent = models.ForeignKey(Choices, related_name='answer_name', on_delete=models.CASCADE)

	def __str__(self):
		return self.answer_text 

# === POST models ===

class AnswerText(models.Model):
	answer = models.AutoField(primary_key=True)
	user = models.IntegerField()
	answer_text = models.CharField(max_length=200)
	question_parent = models.ForeignKey(Questions, on_delete=models.CASCADE)

	def __str__(self):
		return self.answer_text 

class AnswerOne(models.Model):
	answer = models.AutoField(primary_key=True)
	user = models.IntegerField()
	choice_parent = models.ForeignKey(Choices, on_delete=models.CASCADE)

	def __str__(self):
		return str(self.choice_parent) 

class AnswerChoice(models.Model):
	answer = models.AutoField(primary_key=True)
	user = models.IntegerField()
	choice_parent = models.ForeignKey(Choices, on_delete=models.CASCADE)

	def __str__(self):
		return str(self.choice_parent) 

