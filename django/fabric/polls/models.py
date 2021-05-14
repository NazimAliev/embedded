from django.db import models

# Create your models here.
class Poller(models.Model):
	poller_id = models.AutoField(primary_key=True)
	poller_title = models.CharField(max_length=50)
	poller_text = models.CharField(max_length=200)
	start_date = models.DateTimeField('start_date')
	end_date = models.DateTimeField('end_date')
	def __str__(self):
		return self.poller_title

class Question(models.Model):
	question_id = models.AutoField(primary_key=True)
	poller = models.ForeignKey(Poller, on_delete=models.CASCADE)
	question_text = models.CharField(max_length=200)
	question_type = models.IntegerField()
	def __str__(self):
		return str(self.poller) + ' => ' + self.question_text

class Choice(models.Model):
	choice_id = models.AutoField(primary_key=True)
	question = models.ForeignKey(Question, on_delete=models.CASCADE)
	choice_text = models.CharField(max_length=200)
	def __str__(self):
		return str(self.question) + ' => ' + self.choice_text

class AnswerText(models.Model):
	answer_id = models.AutoField(primary_key=True)
	user_id = models.IntegerField()
	question = models.ForeignKey(Question, on_delete=models.CASCADE)
	answer_text = models.CharField(max_length=200)
	def __str__(self):
		return str(self.question) + ' => ' + self.answer_text

class AnswerChoice(models.Model):
	answer_id = models.AutoField(primary_key=True)
	user_id = models.IntegerField()
	choice = models.ForeignKey(Choice, on_delete=models.CASCADE)
	answer_choice = models.IntegerField()
	def __str__(self):
		return str(self.choice)

